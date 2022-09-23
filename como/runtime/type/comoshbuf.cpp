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
 * Copyright (C) 2005 The Android Open Source Project
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

#include "ComoConfig.h"
#include "comoshbuf.h"
#include "util/comolog.h"
#include <cstdlib>
#include <cstring>

namespace como {

SharedBuffer* SharedBuffer::Alloc(
    /* [in] */ size_t size)
{
    // Don't overflow if the combined size of the buffer / header is larger than
    // size_max.
    if (size >= (SIZE_MAX - sizeof(SharedBuffer))) {
        Logger::E("SharedBuffer", "Invalid buffer size %zu", size);
        return nullptr;
    }

    SharedBuffer *sb;
#ifdef COMO_FUNCTION_SAFETY_RTOS
    sb = static_cast<SharedBuffer *>(MemPoolAlloc(sizeof(SharedBuffer) + size));
#else
    sb = static_cast<SharedBuffer *>(malloc(sizeof(SharedBuffer) + size));
#endif
    if (nullptr != sb) {
        // Should be std::atomic_init(&sb->mRefs, 1);
        // But that generates a warning with some compilers.
        // The following is OK on Android-supported platforms.
        sb->mRefs.store(1, std::memory_order_relaxed);
        sb->mSize = size;
        sb->mCapacity = size;
    }
    return sb;
}

void SharedBuffer::Dealloc(
    /* [in] */ const SharedBuffer* released)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    MemPoolFree(0, const_cast<SharedBuffer*>(released));
#else
    free(const_cast<SharedBuffer*>(released));
#endif
}

SharedBuffer* SharedBuffer::Edit() const
{
    if (OnlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    SharedBuffer* sb = Alloc(mSize);
    if (nullptr != sb) {
        memcpy(sb->GetData(), GetData(), GetSize());
        Release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::EditResize(
    /* [in] */ size_t newSize) const
{
    if (OnlyOwner()) {
        SharedBuffer* buf = const_cast<SharedBuffer*>(this);
        if (buf->mSize == newSize)
            return buf;
        // Don't overflow if the combined size of the new buffer / header is larger than
        // size_max.
        if (newSize >= (SIZE_MAX - sizeof(SharedBuffer))) {
            Logger::E("SharedBuffer", "Invalid buffer size %zu", newSize);
            return nullptr;
        }

        buf = (SharedBuffer*)realloc(buf, sizeof(SharedBuffer) + newSize);
        if (nullptr != buf) {
            buf->mSize = newSize;
            buf->mCapacity = newSize;
            return buf;
        }
    }
    SharedBuffer* sb = Alloc(newSize);
    if (nullptr != sb) {
        memcpy(sb->GetData(), GetData(), newSize < mSize ? newSize : mSize);
        Release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::AttemptEdit() const
{
    if (OnlyOwner()) {
        return const_cast<SharedBuffer*>(this);
    }
    return 0;
}

SharedBuffer* SharedBuffer::Reset(
    /* [in] */ size_t new_size) const
{
    SharedBuffer* sb = Alloc(new_size);
    if (nullptr != sb) {
        Release();
    }
    return sb;
}

SharedBuffer* SharedBuffer::Reserve(
        /* [in] */ size_t newCapacity) const
{
    if (OnlyOwner()) {
        SharedBuffer* buf = const_cast<SharedBuffer*>(this);
        if (buf->mSize == newCapacity)
            return buf;
        // Don't overflow if the combined size of the new buffer / header is larger than
        // size_max.
        if (newCapacity >= (SIZE_MAX - sizeof(SharedBuffer))) {
            Logger::E("SharedBuffer", "Invalid buffer size %zu", newCapacity);
            return nullptr;
        }

        buf = (SharedBuffer*)realloc(buf, sizeof(SharedBuffer) + newCapacity);
        if (buf != nullptr) {
            if (newCapacity < buf->mSize)
                buf->mSize = newCapacity;

            buf->mCapacity = newCapacity;
            return buf;
        }
    }
    SharedBuffer* sb = Alloc(newCapacity);
    if (nullptr != sb) {
        memcpy(sb->GetData(), GetData(), newCapacity < mSize ? newCapacity : mSize);
        Release();
    }
    return sb;
}

int32_t SharedBuffer::AddRef() const
{
    int32_t prevRefCount = mRefs.fetch_add(1, std::memory_order_relaxed);
    return prevRefCount + 1;
}

int32_t SharedBuffer::Release(
    /* [in] */ uint32_t flags) const
{
    const bool useDealloc = ((flags & eKeepStorage) == 0);
    if (OnlyOwner()) {
        // Since we're the only owner, our reference count goes to zero.
        mRefs.store(0, std::memory_order_relaxed);
        if (useDealloc) {
            Dealloc(this);
        }
        // As the only owner, our previous reference count was 1.
        return 0;
    }
    // There's multiple owners, we need to use an atomic decrement.
    int32_t prevRefCount = mRefs.fetch_sub(1, std::memory_order_release);
    if (prevRefCount == 1) {
        // We're the last reference, we need the acquire fence.
        atomic_thread_fence(std::memory_order_acquire);
        if (useDealloc) {
            Dealloc(this);
        }
    }
    return prevRefCount - 1;
}

CMemPoolSet *SharedBuffer::memPoolSet = new CMemPoolSet(
                ComoConfig::itemsSharedBuffer, ComoConfig::numSharedBuffer,
                ComoConfig::unitNumGeneralBuffer, ComoConfig::unitSizeGeneralBuffer);
void *SharedBuffer::MemPoolAlloc(size_t ulSize)
{
    return memPoolSet->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void SharedBuffer::MemPoolFree(short id, const void *p)
{
    memPoolSet->Free((void *)p);
}

} // namespace como
