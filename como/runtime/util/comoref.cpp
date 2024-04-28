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

/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "comolog.h"
#include "comoref.h"
#include "mutex.h"
#include "comoobj.h"

namespace como {

#define INITIAL_STRONG_VALUE (1<<28)

#define MAX_COUNT 0xFFFFu

#define BAD_STRONG(c) \
        ((c) == 0 || ((c) & (~(MAX_COUNT | INITIAL_STRONG_VALUE))) != 0)

#define BAD_WEAK(c) ((c) == 0 || ((c) & (~MAX_COUNT)) != 0)


class COM_LOCAL RefBase::WeakRefImpl : public RefBase::WeakRef
{

// #if !DEBUG_REFS -------------------------------------------------------------
#if !DEBUG_REFS

public:
    explicit WeakRefImpl(
        /* [in] */ RefBase* base);

    void AddStrongRef(
        /* [in] */ const void* /*id*/);

    void RemoveStrongRef(
        /* [in] */ const void* /*id*/);

    void RenameStrongRefId(
        /* [in] */ const void* /*old_id*/,
        /* [in] */ const void* /*new_id*/);

    void AddWeakRef(
        /* [in] */ const void* /*id*/);

    void RemoveWeakRef(
        /* [in] */ const void* /*id*/);

    void RenameWeakRefId(
        /* [in] */ const void* /*old_id*/,
        /* [in] */ const void* /*new_id*/);

    void PrintRefs(const char* objInfo) const;

    void TrackMe(
        /* [in] */ Boolean,
        /* [in] */ Boolean);

// #else of !DEBUG_REFS --------------------------------------------------------
#else

private:
    struct RefEntry
    {
        RefEntry* mNext;
        const void* mId;
#if DEBUG_REFS_CALLSTACK_ENABLED
        CallStack mStack;
#endif
        Integer mRef;
    };

public:
    explicit WeakRefImpl(
        /* [in] */ RefBase* base);

    ~WeakRefImpl();

    void AddStrongRef(
        /* [in] */ const void* id);

    void RemoveStrongRef(
        /* [in] */ const void* id);

    void RenameStrongRefId(
        /* [in] */ const void* oldId,
        /* [in] */ const void* newId);

    void AddWeakRef(
        /* [in] */ const void* id);

    void RemoveWeakRef(
        /* [in] */ const void* id);

    void RenameWeakRefId(
        /* [in] */ const void* oldId,
        /* [in] */ const void* newId);

    void PrintRefs(const char* objInfo) const;

    void TrackMe(
        /* [in] */ Boolean,
        /* [in] */ Boolean);

private:
    void AddRef(
        /* [in] */ RefEntry** refs,
        /* [in] */ const void* id,
        /* [in] */ Integer ref);

    void RemoveRef(
        /* [in] */ RefEntry** refs,
        /* [in] */ const void* id);

    void RenameRefsId(
        /* [in] */ RefEntry* r,
        /* [in] */ const void* oldId,
        /* [in] */ const void* newId);

    void PrintRefsLocked(
        /* [out] */ String* out,
        /* [in] */ const RefEntry* refs) const;

// #endif !DEBUG_REFS ----------------------------------------------------------
#endif // !DEBUG_REFS

public:
    std::atomic<Integer> mStrong;
    std::atomic<Integer> mWeak;
    RefBase* const mBase;
    std::atomic<Integer> mFlags;

#if DEBUG_REFS

private:
    mutable Mutex mMutex;
    RefEntry* mStrongRefs;
    RefEntry* mWeakRefs;

    Boolean mTrackEnabled;
    Boolean mRetain;

#endif
};

// #if !DEBUG_REFS -------------------------------------------------------------
#if !DEBUG_REFS

RefBase::WeakRefImpl::WeakRefImpl(
    /* [in] */ RefBase* base)
    : mStrong(INITIAL_STRONG_VALUE)
    , mWeak(0)
    , mBase(base)
    , mFlags(0)
{}

void RefBase::WeakRefImpl::AddStrongRef(
    /* [in] */ const void* /*id*/)
{}

void RefBase::WeakRefImpl::RemoveStrongRef(
    /* [in] */ const void* /*id*/)
{}

void RefBase::WeakRefImpl::RenameStrongRefId(
    /* [in] */ const void* /*old_id*/,
    /* [in] */ const void* /*new_id*/)
{}

void RefBase::WeakRefImpl::AddWeakRef(
    /* [in] */ const void* /*id*/)
{}

void RefBase::WeakRefImpl::RemoveWeakRef(
    /* [in] */ const void* /*id*/)
{}

void RefBase::WeakRefImpl::RenameWeakRefId(
    /* [in] */ const void* /*old_id*/,
    /* [in] */ const void* /*new_id*/)
{}

void RefBase::WeakRefImpl::PrintRefs(const char* objInfo) const
{}

void RefBase::WeakRefImpl::TrackMe(
    /* [in] */ Boolean track,
    /* [in] */ Boolean retain)
{}

// #else !DEBUG_REFS -----------------------------------------------------------
#else

RefBase::WeakRefImpl::WeakRefImpl(
    /* [in] */ RefBase* base)
    : mStrong(INITIAL_STRONG_VALUE)
    , mWeak(0)
    , mBase(base)
    , mFlags(0)
    , mStrongRefs(nullptr)
    , mWeakRefs(nullptr)
    , mTrackEnabled(!!DEBUG_REFS_ENABLED_BY_DEFAULT)
    , mRetain(false)
{
}

RefBase::WeakRefImpl::~WeakRefImpl()
{
#if DEBUG_REFS
    Boolean dumpStack = false;
    char buf[128];

    snprintf(buf, sizeof(buf), "\nRefBase:%p\n", mBase);
    String text = buf;

    if ((! mRetain) && (mStrongRefs != nullptr)) {
        dumpStack = true;
        text += "Strong references remain:\n";
        RefEntry* refs = mStrongRefs;
        while (refs != nullptr) {
            char inc = ((refs->mRef >= 0) ? '+' : '-');
            snprintf(buf, sizeof(buf),
                      "\t%c ID %p (refCount %d)\n", inc, refs->mId, refs->mRef);
            text += buf;
  #if DEBUG_REFS_CALLSTACK_ENABLED
            refs->mStack.log("RefBase");
  #endif
            refs = refs->mNext;
        }
    }

    if ((! mRetain) && (mWeakRefs != nullptr)) {
        dumpStack = true;
        text += "Weak references remain:\n";
        RefEntry* refs = mWeakRefs;
        while (refs != nullptr) {
            char inc = ((refs->mRef >= 0) ? '+' : '-');
            snprintf(buf, sizeof(buf),
                      "\t%c ID %p (refCount %d)\n", inc, refs->mId, refs->mRef);
            text += buf;
  #if DEBUG_REFS_CALLSTACK_ENABLED
            refs->mStack.log("RefBase");
  #endif
            refs = refs->mNext;
        }
    }
    if (dumpStack) {
  #if DEBUG_REFS_CALLSTACK_ENABLED
        text += "RefBase", "above errors at:\n";
        CallStack stack("RefBase");
  #endif
    }

    if (text.GetByteLength() > 1) {
        Logger::D("RefBase::WeakRefImpl::~WeakRefImpl", text.string());
    }
#endif
}

void RefBase::WeakRefImpl::AddStrongRef(
    /* [in] */ const void* id)
{
    AddRef(&mStrongRefs, id, mStrong.load(std::memory_order_relaxed));
}

void RefBase::WeakRefImpl::RemoveStrongRef(
    /* [in] */ const void* id)
{
    if (! mRetain) {
        RemoveRef(&mStrongRefs, id);
    }
    else {
        AddRef(&mStrongRefs, id, -mStrong.load(std::memory_order_relaxed));
    }
}

void RefBase::WeakRefImpl::RenameStrongRefId(
    /* [in] */ const void* oldId,
    /* [in] */ const void* newId)
{
    RenameRefsId(mStrongRefs, oldId, newId);
}

void RefBase::WeakRefImpl::AddWeakRef(
    /* [in] */ const void* id)
{
    AddRef(&mWeakRefs, id, mWeak.load(std::memory_order_relaxed));
}

void RefBase::WeakRefImpl::RemoveWeakRef(
    /* [in] */ const void* id)
{
    if (! mRetain) {
        RemoveRef(&mWeakRefs, id);
    }
    else {
        AddRef(&mWeakRefs, id, -mWeak.load(std::memory_order_relaxed));
    }
}

void RefBase::WeakRefImpl::RenameWeakRefId(
    /* [in] */ const void* oldId,
    /* [in] */ const void* newId)
{
    RenameRefsId(mWeakRefs, oldId, newId);
}

void RefBase::WeakRefImpl::PrintRefs(const char* objInfo) const
{
    String text;
    char buf[128];

    Mutex::AutoLock lock(mMutex);

    snprintf(buf, sizeof(buf),
        "Strong references [%s, reference count: %d] on RefBase %p (WeakRef %p):\n",
        objInfo, mBase->GetStrongCount(), mBase, this);
    text += buf;
    PrintRefsLocked(&text, mStrongRefs);
    snprintf(buf, sizeof(buf),
        "Weak references [%s, reference count: %d] on RefBase %p (WeakRef %p):\n",
        objInfo, GetWeakCount(), mBase, this);
    text += buf;
    PrintRefsLocked(&text, mWeakRefs);

    Logger::D("RefBase", "STACK TRACE for %p\n%s", this, text.string());
}

void RefBase::WeakRefImpl::TrackMe(
    /* [in] */ Boolean track,
    /* [in] */ Boolean retain)
{
    mTrackEnabled = track;
    mRetain = retain;
}

void RefBase::WeakRefImpl::AddRef(
    /* [in] */ RefEntry** refs,
    /* [in] */ const void* id,
    /* [in] */ Integer refCount)
{
    if (mTrackEnabled) {
        Mutex::AutoLock lock(mMutex);

        RefEntry* ref = new RefEntry();
        if (nullptr == ref) {
            Logger::E("RefBase", "new RefEntry() == nullptr");
            *refs = nullptr;
            return;
        }

        ref->mRef = refCount;
        ref->mId = id;
#if DEBUG_REFS_CALLSTACK_ENABLED
        ref->mStack.update(2);
#endif
        ref->mNext = *refs;
        *refs = ref;
    }
}

void RefBase::WeakRefImpl::RemoveRef(
    /* [in] */ RefEntry** refs,
    /* [in] */ const void* id)
{
    // The value of mTrackEnabled may be changed at any time during the object
    // life cycle and cannot be used as a basis for determining whether to
    // delete *refs
    if (*refs != nullptr) {
        Mutex::AutoLock lock(mMutex);

        RefEntry* refTmp = nullptr;
        RefEntry* ref = *refs;
        while (ref != nullptr) {
            if (ref->mId == id) {
                if (nullptr != refTmp) {
                    refTmp->mNext = ref->mNext;
                }
                else {
                    *refs = ref->mNext;
                }
                delete ref;
                return;
            }
            refTmp = ref;
            ref = ref->mNext;
        }

#if DEBUG_REFS
        if (mStrong.load(std::memory_order_relaxed) > 1) {
            String text;
            char buf[128];

            snprintf(buf, sizeof(buf),
                     "removing id %p on RefBase %p (WeakRef %p), it doesn't exist!\n",
                     id, mBase, this);
            text += buf;

            ref = *refs;
            while (ref != nullptr) {
                char inc = ((ref->mRef >= 0) ? '+' : '-');
                snprintf(buf, sizeof(buf),
                                 "\t%c ID %p (refCount %d)\n", inc, ref->mId, ref->mRef);
                text += buf;

                ref = ref->mNext;
            }

          #if DEBUG_REFS_CALLSTACK_ENABLED
            CallStack stack(LOG_TAG);
          #endif
            Logger::D("RefBase::WeakRefImpl::RemoveRef", text.string());
        }
#endif
    }
}

void RefBase::WeakRefImpl::RenameRefsId(
    /* [in] */ RefEntry* r,
    /* [in] */ const void* oldId,
    /* [in] */ const void* newId)
{
    if (mTrackEnabled) {
        Mutex::AutoLock lock(mMutex);

        RefEntry* ref = r;
        while (ref != nullptr) {
            if (ref->mId == oldId) {
                ref->mId = newId;
            }
            ref = ref->mNext;
        }
    }
}

void RefBase::WeakRefImpl::PrintRefsLocked(
    /* [out] */ String* out,
    /* [in] */ const RefEntry* refs) const
{
    char buf[128];
    while (refs != nullptr) {
        char inc = ((refs->mRef >= 0) ? '+' : '-');
        snprintf(buf, sizeof(buf), "\t%c ID %p (ref %d):\n",
                                           inc, refs->mId, refs->mRef);
        *out += buf;
#if DEBUG_REFS_CALLSTACK_ENABLED
        *out += refs->stack.toString("\t\t");
#else
        *out += "\t\t(call stacks disabled)";
#endif
        refs = refs->mNext;
    }
}

// #endif !DEBUG_REFS ----------------------------------------------------------
#endif // !DEBUG_REFS


Integer RefBase::IncStrong(
    /* [in] */ const void* id) const
{
    WeakRefImpl* const refs = mRefs;
    refs->IncWeak(id);

    refs->AddStrongRef(id);
    const Integer cnt = refs->mStrong.fetch_add(1, std::memory_order_relaxed);
    if (cnt <= 0) {
        Logger::E("RefBase", "IncStrong() called on %p after last strong ref",
                                                                          refs);
        assert(0);
    }

    if (cnt != INITIAL_STRONG_VALUE)  {
#if PRINT_REFS
        Logger::D("RefBase", "IncStrong, RefBase:%p id:%p mCount:%d\n", this,
                                                                    id, (cnt+1));
#endif
        return (cnt + 1);
    }

    Integer old = refs->mStrong.fetch_sub(INITIAL_STRONG_VALUE,
                                                    std::memory_order_relaxed);
    if (old <= INITIAL_STRONG_VALUE) {
        Logger::E("RefBase", "mCount:0x%x too small", old);
        assert(0);
    }
    refs->mBase->OnFirstRef(id);

#if PRINT_REFS
    Logger::D("RefBase", "IncStrong, RefBase:%p id:%p mCount:1\n", this, id);
#endif
    return 1;
}

Integer RefBase::DecStrong(
    /* [in] */ const void* id) const
{
    WeakRefImpl* const refs = mRefs;
    refs->RemoveStrongRef(id);

    const Integer cnt = refs->mStrong.fetch_sub(1, std::memory_order_release);
    if (BAD_STRONG(cnt)) {
        Logger::E("RefBase", "DecStrong() called on %p too many times", refs);
        assert(0);
    }

    if (1 == cnt) {
        refs->mBase->OnLastStrongRef(id);

        std::atomic_thread_fence(std::memory_order_acquire);
        Integer flags = refs->mFlags.load(std::memory_order_relaxed);
        if ((flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            if (LIKELY(0 == funFreeMem)) {
                delete this;
            }
            else {
                FREE_MEM_FUNCTION func;
                func = reinterpret_cast<FREE_MEM_FUNCTION>(funFreeMem & 0xFFFFFFFFFFFF);
                                                                        // 5 4 3 2 1 0
                Short shortPara = (Short)(funFreeMem >> 48);
                this->~RefBase();

                /**
                 * One can use this design to allow the owner of an object to
                 * release memory in a controlled manner, for example, when the
                 * memory is sourced from a memory pool.
                 */
                if (LIKELY(nullptr != func)) {
                    func(shortPara, reinterpret_cast<const char *>(this) - OBJECTSIZE_Offset);
                }
            }
        }
    }

#if PRINT_REFS
    Logger::D("RefBase", "DecStrong, RefBase:%p id:%p mCount:%d\n", this, id, (cnt-1));
#endif
    refs->DecWeak(id);

    return (cnt - 1);
}

Integer RefBase::ForceIncStrong(
    /* [in] */ const void* id) const
{
    // Allows initial mStrong of 0 in addition to INITIAL_STRONG_VALUE.
    WeakRefImpl* const refs = mRefs;
    refs->IncWeak(id);

    refs->AddStrongRef(id);
    const Integer c = refs->mStrong.fetch_add(1, std::memory_order_relaxed);
    if (c < 0) {
        Logger::E("RefBase", "forceIncStrong called on %p after ref count underflow",
                                                                                refs);
        assert(0);
    }
#if PRINT_REFS
    Logger::D("RefBase", "ForceIncStrong of %p from %p: cnt=%d\n", this, id, c);
#endif

    switch (c) {
        case INITIAL_STRONG_VALUE:
            refs->mStrong.fetch_sub(INITIAL_STRONG_VALUE,
                                                    std::memory_order_relaxed);
            // fall through...
        case 0:
            refs->mBase->OnFirstRef(id);
            return 1;
        default:
            return (c + 1);
    }

}

Integer RefBase::GetStrongCount() const
{
    return mRefs->mStrong.load(std::memory_order_relaxed);
}

RefBase* RefBase::WeakRef::GetRefBase() const
{
    return static_cast<const WeakRefImpl*>(this)->mBase;
}

void RefBase::WeakRef::IncWeak(
    /* [in] */ const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);
    impl->AddWeakRef(id);
    const Integer c = impl->mWeak.fetch_add(1, std::memory_order_relaxed);
    if (c < 0) {
        Logger::E("RefBase", "IncWeak called on %p after last weak ref", this);
        assert(0);
    }
}

void RefBase::WeakRef::DecWeak(
    /* [in] */ const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);
    impl->RemoveWeakRef(id);
    const Integer c = impl->mWeak.fetch_sub(1, std::memory_order_release);
    if (BAD_WEAK(c)) {
        Logger::E("RefBase", "decWeak called on %p too many times", this);
        assert(0);
    }
    if (c != 1) {
        return;
    }

    atomic_thread_fence(std::memory_order_acquire);
    Integer flags = impl->mFlags.load(std::memory_order_relaxed);
    if ((flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
        // This is the regular lifetime case. The object is destroyed
        // when the last strong reference goes away. Since weakref_impl
        // outlives the object, it is not destroyed in the dtor, and
        // we'll have to do it here.
        if (impl->mStrong.load(std::memory_order_relaxed)
                                                        == INITIAL_STRONG_VALUE) {
            // Decrementing a weak count to zero when object never had a strong
            // reference.  We assume it acquired a weak reference early, e.g.
            // in the constructor, and will eventually be properly destroyed,
            // usually via incrementing and decrementing the strong count.
            // Thus we no longer do anything here.  We log this case, since it
            // seems to be extremely rare, and should not normally occur. We
            // used to deallocate mBase here, so this may now indicate a leak.
            Logger::W("RefBase", "Object at %p lost last weak reference "
                                "before it had a strong reference", impl->mBase);
        }
        else {
            // ALOGV("Freeing refs %p of old RefBase %p\n", this, impl->mBase);
            delete impl;
        }
    }
    else {
        // This is the OBJECT_LIFETIME_WEAK case. The last weak-reference
        // is gone, we can destroy the object.
        impl->mBase->OnLastWeakRef(id);
        delete impl->mBase;
    }
}

Boolean RefBase::WeakRef::AttemptIncStrong(
    /* [in] */ const void* id)
{
    IncWeak(id);

    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);
    Integer curCount = impl->mStrong.load(std::memory_order_relaxed);

    if (curCount < 0) {
        Logger::E("RefBase", "AttemptIncStrong called on %p after underflow", this);
        assert(0);
    }

    while ((curCount > 0) && (curCount != INITIAL_STRONG_VALUE)) {
        // we're in the easy/common case of promoting a weak-reference
        // from an existing strong reference.
        if (impl->mStrong.compare_exchange_weak(curCount, curCount + 1,
                std::memory_order_relaxed)) {
            break;
        }
        // the strong count has changed on us, we need to re-assert our
        // situation. curCount was updated by compare_exchange_weak.
    }

    if ((curCount <= 0) || (curCount == INITIAL_STRONG_VALUE)) {
        // we're now in the harder case of either:
        // - there never was a strong reference on us
        // - or, all strong references have been released
        Integer flags = impl->mFlags.load(std::memory_order_relaxed);
        if ((flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            // this object has a "normal" life-time, i.e.: it gets destroyed
            // when the last strong reference goes away
            if (curCount <= 0) {
                // the last strong-reference got released, the object cannot
                // be revived.
                DecWeak(id);
                return false;
            }

            // here, curCount == INITIAL_STRONG_VALUE, which means
            // there never was a strong-reference, so we can try to
            // promote this object; we need to do that atomically.
            while (curCount > 0) {
                if (impl->mStrong.compare_exchange_weak(curCount, curCount + 1,
                        std::memory_order_relaxed)) {
                    break;
                }
                // the strong count has changed on us, we need to re-assert our
                // situation (e.g.: another thread has inc/decStrong'ed us)
                // curCount has been updated.
            }

            if (curCount <= 0) {
                // promote() failed, some other thread destroyed us in the
                // meantime (i.e.: strong count reached zero).
                DecWeak(id);
                return false;
            }
        }
        else {
            // this object has an "extended" life-time, i.e.: it can be
            // revived from a weak-reference only.
            // Ask the object's implementation if it agrees to be revived
            if (! impl->mBase->OnIncStrongAttempted(FIRST_INC_STRONG, id)) {
                // it didn't so give-up.
                DecWeak(id);
                return false;
            }
            // grab a strong-reference, which is always safe due to the
            // extended life-time.
            curCount = impl->mStrong.fetch_add(1, std::memory_order_relaxed);
            // If the strong reference count has already been incremented by
            // someone else, the implementor of onIncStrongAttempted() is holding
            // an unneeded reference.  So call onLastStrongRef() here to remove it.
            // (No, this is not pretty.)  Note that we MUST NOT do this if we
            // are in fact acquiring the first reference.
            if ((curCount != 0) && (curCount != INITIAL_STRONG_VALUE)) {
                impl->mBase->OnLastStrongRef(id);
            }
        }
    }

    impl->AddStrongRef(id);

#if PRINT_REFS
    Logger::D("RefBase", "attemptIncStrong of %p from %p: cnt=%d\n", this, id, curCount);
#endif

    // curCount is the value of mStrong before we incremented it.
    // Now we need to fix-up the count if it was INITIAL_STRONG_VALUE.
    // This must be done safely, i.e.: handle the case where several threads
    // were here in attemptIncStrong().
    // curCount > INITIAL_STRONG_VALUE is OK, and can happen if we're doing
    // this in the middle of another incStrong.  The subtraction is handled
    // by the thread that started with INITIAL_STRONG_VALUE.
    if (curCount == INITIAL_STRONG_VALUE) {
        impl->mStrong.fetch_sub(INITIAL_STRONG_VALUE, std::memory_order_relaxed);
    }

    return true;
}

Boolean RefBase::WeakRef::AttemptIncWeak(
    /* [in] */ const void* id)
{
    WeakRefImpl* const impl = static_cast<WeakRefImpl*>(this);

    Integer curCount = impl->mWeak.load(std::memory_order_relaxed);
    if (curCount < 0) {
        Logger::E("RefBase", "AttemptIncWeak called on %p after underflow", this);
        assert(0);
    }
    while (curCount > 0) {
        if (impl->mWeak.compare_exchange_weak(curCount, curCount + 1,
                                                    std::memory_order_relaxed)) {
            break;
        }
        // curCount has been updated.
    }

    if (curCount > 0) {
        impl->AddWeakRef(id);
    }

    return curCount > 0;
}

Integer RefBase::WeakRef::GetWeakCount() const
{
    return static_cast<const WeakRefImpl*>(this)->mWeak.load(
                                                        std::memory_order_relaxed);
}

void RefBase::WeakRef::PrintRefs(const char* objInfo) const
{
    static_cast<const WeakRefImpl*>(this)->PrintRefs(objInfo);
}

void RefBase::WeakRef::TrackMe(
    /* [in] */ Boolean enable,
    /* [in] */ Boolean retain)
{
    static_cast<WeakRefImpl*>(this)->TrackMe(enable, retain);
}

RefBase::WeakRef* RefBase::CreateWeak(
    /* [in] */ const void* id) const
{
    mRefs->IncWeak(id);
    return mRefs;
}

RefBase::WeakRef* RefBase::GetWeakRefs() const
{
    return mRefs;
}

RefBase::RefBase()
    : mRefs(new WeakRefImpl(this))  // const member should be initialized
    , funFreeMem(0)
{
    if (nullptr == mRefs) {
        Logger::E("RefBase", "Out of memory");
    }
}

RefBase::~RefBase()
{
    Integer flags = mRefs->mFlags.load(std::memory_order_relaxed);
    // Life-time of this object is extended to WEAK, in
    // which case weakref_impl doesn't out-live the object and we
    // can free it now.
    if ((flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK) {
        // It's possible that the weak count is not 0 if the object
        // re-acquired a weak reference in its destructor
        if (mRefs->mWeak.load(std::memory_order_relaxed) == 0) {
            delete mRefs;
        }
    }
    else if (mRefs->mStrong.load(std::memory_order_relaxed)
                                                    == INITIAL_STRONG_VALUE) {
        // We never acquired a strong reference on this object.
        delete mRefs;
    }
    const_cast<WeakRefImpl*&>(mRefs) = nullptr;
}

void RefBase::ExtendObjectLifetime(
    /* [in] */ Integer mode)
{
    // Must be happens-before ordered with respect to construction or any
    // operation that could destroy the object.
    mRefs->mFlags.fetch_or(mode, std::memory_order_relaxed);
}

void RefBase::OnFirstRef(
    /* [in] */ const void* /*id*/)
{}

void RefBase::OnLastStrongRef(
    /* [in] */ const void* /*id*/)
{}

Boolean RefBase::OnIncStrongAttempted(
    /* [in] */ Integer flags,
    /* [in] */ const void* /*id*/)
{
    return (flags & FIRST_INC_STRONG) ? true : false;
}

void RefBase::OnLastWeakRef(
    /* [in] */ const void* /*id*/)
{}

/**
 * Weak reference objects, which do not prevent their referents from executing
 * destructor, and then reclaimed. Weak references are most often used to
 * implement canonicalizing mappings.
 *
 * Suppose that the garbage collector determines at a certain point in time that
 * an object is weakly reachable. At that time it will atomically clear all weak
 * references to that object and all weak references to any other weakly-reachable
 * objects from which that object is reachable through a chain of strong and soft
 * references. At the same time it will declare all of the formerly
 * weakly-reachable objects to be finalizable. At the same time or at some later
 * time it will enqueue those newly-cleared weak references that are registered
 * with reference queues.
 */

WeakReferenceImpl::WeakReferenceImpl(
    /* [in] */ IInterface* object,
    /* [in] */ RefBase::WeakRef* ref)
    : mObject(object)
    , mRef(ref)
{}

WeakReferenceImpl::~WeakReferenceImpl()
{
    if (mRef != nullptr) {
        mRef->DecWeak(this);
    }
}

Integer WeakReferenceImpl::AddRef(
    /* [in] */ HANDLE id)
{
    return LightRefBase::AddRef(id);
}

Integer WeakReferenceImpl::Release(
    /* [in] */ HANDLE id)
{
    return LightRefBase::Release(id);
}

IInterface* WeakReferenceImpl::Probe(
    /* [in] */ const InterfaceID& iid)
{
    if (iid == IID_IInterface) {
        return (IInterface*)(IWeakReference*)this;
    }
    else if (iid == IID_IWeakReference) {
        return (IWeakReference*)this;
    }

    return nullptr;
}

ECode WeakReferenceImpl::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [in] */ InterfaceID& iid)
{
    if (object == (IInterface*)(IWeakReference*)this) {
        iid = IID_IWeakReference;
    }
    else {
        // Just want to assign a value to iid
        iid = IID_IInterface;

        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return NOERROR;
}

ECode WeakReferenceImpl::Resolve(
    /* [in] */ const InterfaceID& iid,
    /* [out] */ IInterface** object)
{
    if ((mObject != nullptr) && mRef->AttemptIncStrong(object)) {
        *object = mObject->Probe(iid);
        if (nullptr == *object) {
            mObject->Release();
        }
    }
    else {
        *object = nullptr;
    }
    return NOERROR;
}

/**
 * class LightRefBase
 */
LightRefBase::LightRefBase()
    : mCount(0)
    , funFreeMem(0)
{
#if DEBUG_REFS_LightRefBase
    mRefs = nullptr;
    mTrackEnabled = false;
    mRetain = false;
#endif
}

LightRefBase::~LightRefBase()
{
#if DEBUG_REFS_LightRefBase
    if (mTrackEnabled) {
        if ((! mRetain) && (mRefs != nullptr)) {
            String text;
            char buf[128];

            snprintf(buf, sizeof(buf),
                "References remain [reference count: %d] on LightRefBase %p:\n",
                mCount.load(std::memory_order_relaxed), this);
            text += buf;

            RefEntry* refs = mRefs;
            while (refs != nullptr) {
                char inc = ((refs->mRef >= 0) ? '+' : '-');
                snprintf(buf, sizeof(buf), "\t%c ID %p (ref %d)\n",
                                          inc, (void *)(refs->mId), refs->mRef);
                text += buf;

      #if DEBUG_REFS_CALLSTACK_ENABLED
                refs->mStack.log("~LightRefBase");
      #endif
                refs = refs->mNext;
            }

            Logger::D("~LightRefBase", text.string());
        }
    }
#endif
}

Integer LightRefBase::AddRef(
    /* [in] */ HANDLE id)
{
#if !DEBUG_REFS_LightRefBase
    (void)id;
#endif

    Integer c = mCount.fetch_add(1, std::memory_order_relaxed);

#if DEBUG_REFS_LightRefBase
    if (mTrackEnabled) {
        Mutex::AutoLock lock(mMutex);

        RefEntry* ref = new RefEntry();
        if (nullptr == ref) {
            Logger::E("LightRefBase::AddRef", "new RefEntry() == nullptr");
            return (c + 1);
        }

        ref->mRef = mCount.load(std::memory_order_relaxed);
        ref->mId = id;
      #if DEBUG_REFS_CALLSTACK_ENABLED
        ref->mStack.update(2);
      #endif
        ref->mNext = mRefs;
        mRefs = ref;

        Logger::D("LightRefBase::AddRef", "ID %p (ref %d)",
                                                 (void *)(ref->mId), ref->mRef);
    }
#endif

    return (c + 1);
}

Integer LightRefBase::Release(
    /* [in] */ HANDLE id)
{
    Integer cnt = mCount.fetch_sub(1, std::memory_order_release);
    if (1 == cnt) {
#if DEBUG_REFS_LightRefBase
        bool notFoundIt = true;

        if (mTrackEnabled) {
            Mutex::AutoLock lock(mMutex);

            RefEntry* refLast = nullptr;
            RefEntry* ref = mRefs;
            while (ref != nullptr) {
                if (ref->mId == id) {
                    notFoundIt = false;

                    if (nullptr == refLast) {
                        mRefs = ref->mNext;
                    }
                    else {
                        refLast->mNext = ref->mNext;
                        delete ref;
                        break;
                    }
                }
                refLast = ref;
                ref = ref->mNext;
            }

            if (notFoundIt) {
                String text;
                char buf[128];

                snprintf(buf, sizeof(buf),
                            "removing id %p on RefBase %p, it doesn't exist!\n",
                            (void *)id, this);
                text += buf;

                ref = mRefs;
                while (ref != nullptr) {
                    char inc = ((ref->mRef >= 0) ? '+' : '-');
                    snprintf(buf, sizeof(buf), "\t%c ID %p (refCount %d)\n",
                                            inc, (void *)(ref->mId), ref->mRef);
                    text += buf;

                    ref = ref->mNext;
                }

              #if DEBUG_REFS_CALLSTACK_ENABLED
                CallStack stack(LOG_TAG);
              #endif
                Logger::D("LightRefBase::Release", text.string());
            }
        }
#endif

        if (LIKELY(0 == funFreeMem)) {
            delete this;
        }
        else {
            FREE_MEM_FUNCTION func;
            func = reinterpret_cast<FREE_MEM_FUNCTION>(funFreeMem & 0xFFFFFFFFFFFF);
                                                                    // 5 4 3 2 1 0
            Short shortPara = (Short)(funFreeMem >> 48);
            (void)this->~LightRefBase();

            /**
             * One can use this design to allow the owner of an object to
             * release memory in a controlled manner, for example, when the
             * memory is sourced from a memory pool.
             *
             * COMO objects (lifecycle management by count) are defined this way：
             * class C...
             *     : public LightRefBase
             *     , public ...
             *
             * LightRefBase is the first base class, therefore, we must make an
             * adjustment [- sizeof(LightRefBase)] to the current object
             * pointer (this) in order to get the original COMO object pointer.
             */
            if (LIKELY(nullptr != func)) {
                func(shortPara, reinterpret_cast<const char *>(this) - sizeof(LightRefBase));
            }
        }
    }
    return (cnt - 1);
}

IInterface* LightRefBase::Probe(
    /* [in] */ const InterfaceID& iid) const
{
    (void)iid;

    return nullptr;
}

ECode LightRefBase::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID& iid) const
{
    (void)object;
    (void)iid;

    return E_ILLEGAL_ARGUMENT_EXCEPTION;
}

Integer LightRefBase::GetStrongCount() const
{
    return mCount.load(std::memory_order_relaxed);
}

void LightRefBase::SetFunFreeMem(FREE_MEM_FUNCTION func, Short shortPara)
{
    funFreeMem = (static_cast<HANDLE>(shortPara) << 48) |
                              (reinterpret_cast<HANDLE>(func) & 0xFFFFFFFFFFFFu);
                                                                // 5 4 3 2 1 0
}

#if DEBUG_REFS_LightRefBase
void LightRefBase::PrintRefs(const char* objInfo)
{
    String text;
    char buf[128];

    Mutex::AutoLock lock(mMutex);

    snprintf(buf, sizeof(buf),
        "References [%s, reference count: %d] on LightRefBase %p\n",
                    objInfo, mCount.load(std::memory_order_relaxed), this);
    text += buf;

    RefEntry* refs = mRefs;
    while (refs != nullptr) {
        char inc = ((refs->mRef >= 0) ? '+' : '-');
        snprintf(buf, sizeof(buf), "\t%c ID %p (ref %d):\n",
                                          inc, (void *)(refs->mId), refs->mRef);
        text += buf;
  #if DEBUG_REFS_CALLSTACK_ENABLED
        text += refs->stack.toString("\t\t");
  #else
        text += "\t\t(call stacks disabled)";
#endif
        refs = refs->mNext;
    }

    Logger::D("LightRefBase", "%s", text.string());
}

void LightRefBase::TrackMe(
    /* [in] */ Boolean track,
    /* [in] */ Boolean retain)
{
    mTrackEnabled = track;
    mRetain = retain;
}
#endif

} // namespace como
