//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "como/core/nativeapi.h"
#include "como/core/NativeLockWord.h"
#include "como/core/NativeMonitor.h"
#include "como/core/NativeMutex.h"
#include "como/core/NativeScopedThreadStateChange.h"
#include "como/core/NativeThreadList.h"
#include "como/core/NativeTimeUtils.h"
#include <comolog.h>
#include <algorithm>

namespace como {
namespace core {

// Use 0 since we want to yield to prevent blocking for an unpredictable amount of time.
static constexpr useconds_t kThreadSuspendInitialSleepUs = 0;
static constexpr useconds_t kThreadSuspendMaxYieldUs = 3000;
static constexpr useconds_t kThreadSuspendMaxSleepUs = 5000;

NativeThreadList::NativeThreadList(
    /* [in] */ uint64_t threadSuspendTimeoutNs)
    : mSuspendAllCount(0)
    , mDebugSuspendAllCount(0)
    , mUnregisteringCount(0)
    , mThreadSuspendTimeoutNs(threadSuspendTimeoutNs)
{
    CHECK(NativeMonitor::IsValidLockWord(NativeLockWord::FromThinLockId(kMaxThreadId, 1, 0)));
}

Boolean NativeThreadList::Contains(
    /* [in] */ NativeThread* thread)
{
    return find(mList.begin(), mList.end(), thread) != mList.end();
}

// Unlike suspending all threads where we can wait to acquire the mutator_lock_, suspending an
// individual thread requires polling. delay_us is the requested sleep wait. If delay_us is 0 then
// we use sched_yield instead of calling usleep.
static void ThreadSuspendSleep(
    /* [in] */ useconds_t delayUs)
{
    if (delayUs == 0) {
        sched_yield();
    }
    else {
        usleep(delayUs);
    }
}

void NativeThreadList::Resume(
    /* [in] */ NativeThread* thread,
    /* [in] */ Boolean forDebugger)
{
    NativeThread* self = NativeThread::Current();
    CHECK(thread != self);
    Logger::V("NativeThreadList", "Resume(%p) starting...",
            reinterpret_cast<void*>(thread));

    {
        // To check Contains.
        NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
        // To check IsSuspended.
        NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
        CHECK(thread->IsSuspended());
        if (!Contains(thread)) {
            // We only expect threads within the thread-list to have been suspended otherwise we can't
            // stop such threads from delete-ing themselves.
            Logger::E("NativeThreadList", "Resume(%p) thread not within thread list",
                    reinterpret_cast<void*>(thread));
            return;
        }
        Boolean updated = thread->ModifySuspendCount(self, -1, nullptr, forDebugger);
        CHECK(updated);
    }

    {
        Logger::V("NativeThreadList", "Resume(%p) waking others",
                reinterpret_cast<void*>(thread));
        NativeMutex::AutoLock lock(self, *Locks::sThreadSuspendCountLock);
        NativeThread::sResumeCond->Broadcast(self);
    }

    Logger::V("NativeThreadList", "Resume(%p) complete", reinterpret_cast<void*>(thread));
}

NativeThread* NativeThreadList::SuspendThreadByPeer(
    /* [in] */ Thread* peer,
    /* [in] */ Boolean requestSuspension,
    /* [in] */ Boolean debugSuspension,
    /* [out] */ Boolean* timedOut)
{
    const uint64_t startTime = NanoTime();
    useconds_t sleepUs = kThreadSuspendInitialSleepUs;
    *timedOut = false;
    NativeThread* suspendedThread = nullptr;
    NativeThread* const self = NativeThread::Current();
    Logger::V("NativeThreadList", "SuspendThreadByPeer starting");
    while (true) {
        NativeThread* thread;
        {
            // Note: this will transition to runnable and potentially suspend. We ensure only one thread
            // is requesting another suspend, to avoid deadlock, by requiring this function be called
            // holding Locks::thread_list_suspend_thread_lock_. Its important this thread suspend rather
            // than request thread suspension, to avoid potential cycles in threads requesting each other
            // suspend.
            ScopedObjectAccess soa(self);
            NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
            thread = NativeThread::FromManagedThread(peer);
            if (thread == nullptr) {
                if (suspendedThread != nullptr) {
                    NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
                    // If we incremented the suspend count but the thread reset its peer, we need to
                    // re-decrement it since it is shutting down and may deadlock the runtime in
                    // ThreadList::WaitForOtherNonDaemonThreadsToExit.
                    Boolean updated = suspendedThread->ModifySuspendCount(
                            soa.Self(), -1, nullptr, debugSuspension);
                    CHECK(updated);
                }
                Logger::W("NativeThreadList", "No such thread for suspend %p", peer);
                return nullptr;
            }
            if (!Contains(thread)) {
                CHECK(suspendedThread == nullptr);
                Logger::V("NativeThreadList", "SuspendThreadByPeer failed for unattached thread: %p",
                        thread);
                return nullptr;
            }
            Logger::V("NativeThreadList", "SuspendThreadByPeer found thread: %s",
                    thread->ShortDump().string());
            {
                NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
                if (requestSuspension) {
                    if (self->GetSuspendCount() > 0) {
                        // We hold the suspend count lock but another thread is trying to suspend us. Its not
                        // safe to try to suspend another thread in case we get a cycle. Start the loop again
                        // which will allow this thread to be suspended.
                        continue;
                    }
                    CHECK(suspendedThread == nullptr);
                    suspendedThread = thread;
                    Boolean updated = suspendedThread->ModifySuspendCount(self, +1, nullptr, debugSuspension);
                    CHECK(updated);
                    requestSuspension = false;
                }
                else {
                    // If the caller isn't requesting suspension, a suspension should have already occurred.
                    CHECK(thread->GetSuspendCount() > 0);
                }
                // IsSuspended on the current thread will fail as the current thread is changed into
                // Runnable above. As the suspend count is now raised if this is the current thread
                // it will self suspend on transition to Runnable, making it hard to work with. It's simpler
                // to just explicitly handle the current thread in the callers to this code.
                CHECK(thread != self);
                // If thread is suspended (perhaps it was already not Runnable but didn't have a suspend
                // count, or else we've waited and it has self suspended) or is the current thread, we're
                // done.
                if (thread->IsSuspended()) {
                    Logger::V("NativeThreadList", "SuspendThreadByPeer thread suspended: %s",
                            thread->ShortDump().string());
                    return thread;
                }
                const uint64_t totalDelay = NanoTime() - startTime;
                if (totalDelay >= mThreadSuspendTimeoutNs) {
                    Logger::W("NativeThreadList", "Thread suspension timed out %p", peer);
                    if (suspendedThread != nullptr) {
                        CHECK(suspendedThread == thread);
                        Boolean updated = suspendedThread->ModifySuspendCount(
                                soa.Self(), -1, nullptr, debugSuspension);
                        CHECK(updated);
                    }
                    *timedOut = true;
                    return nullptr;
                }
                else if (sleepUs == 0 &&
                    totalDelay > static_cast<uint64_t>(kThreadSuspendMaxYieldUs) * 1000) {
                    // We have spun for kThreadSuspendMaxYieldUs time, switch to sleeps to prevent
                    // excessive CPU usage.
                    sleepUs = kThreadSuspendMaxYieldUs / 2;
                }
            }
        }
        Logger::V("NativeThreadList", "SuspendThreadByPeer waiting to allow thread chance to suspend");
        ThreadSuspendSleep(sleepUs);
        sleepUs = std::min(sleepUs * 2, kThreadSuspendMaxSleepUs);
    }
}

NativeThread* NativeThreadList::SuspendThreadByThreadId(
    /* [in] */ uint32_t threadId,
    /* [in] */ Boolean debugSuspension,
    /* [out] */ Boolean* timedOut)
{
    const uint64_t startTime = NanoTime();
    useconds_t sleepUs = kThreadSuspendInitialSleepUs;
    *timedOut = false;
    NativeThread* suspendedThread = nullptr;
    NativeThread* const self = NativeThread::Current();
    CHECK(threadId != kInvalidThreadId);
    Logger::V("NativeThreadList", "SuspendThreadByThreadId starting");
    while (true) {
        {
            // Note: this will transition to runnable and potentially suspend. We ensure only one thread
            // is requesting another suspend, to avoid deadlock, by requiring this function be called
            // holding Locks::thread_list_suspend_thread_lock_. Its important this thread suspend rather
            // than request thread suspension, to avoid potential cycles in threads requesting each other
            // suspend.
            ScopedObjectAccess soa(self);
            NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
            NativeThread* thread = nullptr;
            for (const auto& it : mList) {
                if (it->GetThreadId() == threadId) {
                    thread = it;
                    break;
                }
            }
            if (thread == nullptr) {
                CHECK(suspendedThread == nullptr);
                // There's a race in inflating a lock and the owner giving up ownership and then dying.
                Logger::W("NativeThreadList", "No such thread id for suspend: %d", threadId);
                return nullptr;
            }
            Logger::V("NativeThreadList", "SuspendThreadByThreadId found thread: %s",
                    thread->ShortDump().string());
            CHECK(Contains(thread));
            {
                NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
                if (suspendedThread == nullptr) {
                    if (self->GetSuspendCount() > 0) {
                        // We hold the suspend count lock but another thread is trying to suspend us. Its not
                        // safe to try to suspend another thread in case we get a cycle. Start the loop again
                        // which will allow this thread to be suspended.
                     continue;
                    }
                    Boolean updated = thread->ModifySuspendCount(self, +1, nullptr, debugSuspension);
                    CHECK(updated);
                    suspendedThread = thread;
                }
                else {
                    CHECK(suspendedThread == thread);
                    // If the caller isn't requesting suspension, a suspension should have already occurred.
                    CHECK(thread->GetSuspendCount() > 0);
                }
                // IsSuspended on the current thread will fail as the current thread is changed into
                // Runnable above. As the suspend count is now raised if this is the current thread
                // it will self suspend on transition to Runnable, making it hard to work with. It's simpler
                // to just explicitly handle the current thread in the callers to this code.
                CHECK(thread != self);
                Logger::V("NativeThreadList", "Attempt to suspend the current thread for the debugger");
                // If thread is suspended (perhaps it was already not Runnable but didn't have a suspend
                // count, or else we've waited and it has self suspended) or is the current thread, we're
                // done.
                if (thread->IsSuspended()) {
                    Logger::V("NativeThreadList", "SuspendThreadByThreadId thread suspended: %s",
                            thread->ShortDump().string());
                    return thread;
                }
                const uint64_t totalDelay = NanoTime() - startTime;
                if (totalDelay >= mThreadSuspendTimeoutNs) {
                    Logger::W("NativeThreadList", "Thread suspension timed out: %d", threadId);
                    if (suspendedThread != nullptr) {
                        Boolean updated = thread->ModifySuspendCount(soa.Self(), -1, nullptr, debugSuspension);
                        CHECK(updated);
                    }
                    *timedOut = true;
                    return nullptr;
                }
                else if (sleepUs == 0 &&
                    totalDelay > static_cast<uint64_t>(kThreadSuspendMaxYieldUs) * 1000) {
                    // We have spun for kThreadSuspendMaxYieldUs time, switch to sleeps to prevent
                    // excessive CPU usage.
                    sleepUs = kThreadSuspendMaxYieldUs / 2;
                }
            }
            // Release locks and come out of runnable state.
        }
        Logger::V("NativeThreadList", "SuspendThreadByThreadId waiting to allow thread chance to suspend");
        ThreadSuspendSleep(sleepUs);
        sleepUs = std::min(sleepUs * 2, kThreadSuspendMaxSleepUs);
    }
}

NativeThread* NativeThreadList::FindThreadByThreadId(
    /* [in] */ uint32_t threadId)
{
    for (const auto& thread : mList) {
        if (thread->GetThreadId() == threadId) {
            return thread;
        }
    }
    return nullptr;
}

void NativeThreadList::Register(
    /* [in] */ NativeThread* self)
{
    CHECK(self == NativeThread::Current());

    Logger::V("NativeThreadList", "ThreadList::Register() %s",
            self->ShortDump().string());

    // Atomically add self to the thread list and make its thread_suspend_count_ reflect ongoing
    // SuspendAll requests.
    NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
    NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
    CHECK(mSuspendAllCount >= mDebugSuspendAllCount);
    // Modify suspend count in increments of 1 to maintain invariants in ModifySuspendCount. While
    // this isn't particularly efficient the suspend counts are most commonly 0 or 1.
    for (int delta = mDebugSuspendAllCount; delta > 0; delta--) {
        Boolean updated = self->ModifySuspendCount(self, +1, nullptr, true);
        CHECK(updated);
    }
    for (int delta = mSuspendAllCount - mDebugSuspendAllCount; delta > 0; delta--) {
        Boolean updated = self->ModifySuspendCount(self, +1, nullptr, false);
        CHECK(updated);
    }
    CHECK(!Contains(self));
    mList.push_back(self);
}

void NativeThreadList::Unregister(
    /* [in] */ NativeThread* self)
{
    CHECK(self == NativeThread::Current());
    CHECK(self->GetState() != kRunnable);
    Locks::sMutatorLock->AssertNotHeld(self);

    Logger::V("NativeThreadList", "ThreadList::Unregister() %s",
            self->ShortDump().string());

    {
        NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
        ++mUnregisteringCount;
    }

    // Any time-consuming destruction, plus anything that can call back into managed code or
    // suspend and so on, must happen at this point, and not in ~Thread. The self->Destroy is what
    // causes the threads to join. It is important to do this after incrementing mUnregisteringCount
    // since we want the runtime to wait for the daemon threads to exit before deleting the thread
    // list.
    self->Destroy();

    uint32_t thinLockId = self->GetThreadId();
    while (true) {
        // Remove and delete the Thread* while holding the thread_list_lock_ and
        // thread_suspend_count_lock_ so that the unregistering thread cannot be suspended.
        // Note: deliberately not using MutexLock that could hold a stale self pointer.
        NativeMutex::AutoLock lock(self, *Locks::sThreadListLock);
        if (!Contains(self)) {
            String threadName;
            self->GetThreadName(&threadName);
            String os;
            DumpNativeStack(&os, GetTid(), nullptr, nullptr);
            Logger::E("NativeThreadList", "Request to unregister unattached thread %s\n%s",
                    threadName.string(), os.string());
            break;
        }
        else {
            NativeMutex::AutoLock lock2(self, *Locks::sThreadSuspendCountLock);
            if (!self->IsSuspended()) {
                mList.remove(self);
                break;
            }
        }
        // We failed to remove the thread due to a suspend request, loop and try again.
    }
    delete self;

    // Release the thread ID after the thread is finished and deleted to avoid cases where we can
    // temporarily have multiple threads with the same thread id. When this occurs, it causes
    // problems in FindThreadByThreadId / SuspendThreadByThreadId.
    ReleaseThreadId(nullptr, thinLockId);

    // Clear the TLS data, so that the underlying native thread is recognizably detached.
    // (It may wish to reattach later.)
#ifdef ART_TARGET_ANDROID
    __get_tls()[TLS_SLOT_ART_THREAD_SELF] = nullptr;
#else
    pthread_setspecific(NativeThread::sPthreadKeySelf, nullptr);
#endif

    // Signal that a thread just detached.
    NativeMutex::AutoLock lock(nullptr, *Locks::sThreadListLock);
    --mUnregisteringCount;
    Locks::sThreadExitCond->Broadcast(nullptr);
}

uint32_t NativeThreadList::AllocThreadId(
    /* [in] */ NativeThread* self)
{
    NativeMutex::AutoLock lock(self, *Locks::sAllocatedThreadIdsLock);
    for (size_t i = 0; i < mAllocatedIds.size(); ++i) {
        if (!mAllocatedIds[i]) {
            mAllocatedIds.set(i);
            return i + 1;  // Zero is reserved to mean "invalid".
        }
    }
    Logger::E("NativeThreadList", "Out of internal thread ids");
    return 0;
}

void NativeThreadList::ReleaseThreadId(
    /* [in] */ NativeThread* self,
    /* [in] */ uint32_t id)
{
    NativeMutex::AutoLock lock(self, *Locks::sAllocatedThreadIdsLock);
    --id;  // Zero is reserved to mean "invalid".
    CHECK(mAllocatedIds[id]);
    mAllocatedIds.reset(id);
}

}
}
