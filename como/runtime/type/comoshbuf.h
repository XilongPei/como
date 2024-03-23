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

#ifndef __COMO_SHAREDBUFFER_H__
#define __COMO_SHAREDBUFFER_H__

#include "comodef.h"
#include "MemPool.h"
#include <atomic>
#include <cstddef>

namespace como {

class COM_PUBLIC SharedBuffer
{
public:
    /* flags to use with release() */
    enum {
        eKeepStorage = 0x00000001u
    };

    /*! allocate a buffer of size 'size' and acquire() it.
     *  call release() to free it.
     */
    static SharedBuffer* Alloc(
        /* [in] */ size_t size);

    /*! free the memory associated with the SharedBuffer.
     * Fails if there are any users associated with this SharedBuffer.
     * In other words, the buffer must have been release by all its
     * users.
     */
    static void Dealloc(
        /* [in] */ const SharedBuffer* released);

    //! access the data for read
    inline const void* GetData() const;

    //! access the data for read/write
    inline void* GetData();

    //! get size of the buffer
    inline size_t GetSize() const;

    //! set size of the buffer
    inline size_t SetSize(size_t size);

    //! get back a SharedBuffer object from its data
    inline static SharedBuffer* GetBufferFromData(
        /* [in] */ void* data);

    //! get back a SharedBuffer object from its data
    inline static const SharedBuffer* GetBufferFromData(
        /* [in] */ const void* data);

    //! get the size of a SharedBuffer object from its data
    inline static size_t GetSizeFromData(
        /* [in] */ const void* data);

    //! edit the buffer (get a writtable, or non-const, version of it)
    SharedBuffer* Edit() const;

    //! edit the buffer, resizing if needed
    SharedBuffer* EditResize(
        /* [in] */ size_t size) const;

    //! like edit() but fails if a copy is required
    SharedBuffer* AttemptEdit() const;

    //! resize and edit the buffer, loose it's content.
    SharedBuffer* Reset(
        /* [in] */ size_t size) const;

    /**
     * Request a change in capacity
     * Requests that the string capacity be adapted to a planned change in size
     * to a length of up to n characters.
     * If n is greater than the current string capacity, the function causes the
     * container to increase its capacity to n characters (or greater).
     * In all other cases, it is taken as a non-binding request to shrink the
     * string capacity: the container implementation is free to optimize otherwise
     * and leave the string with a capacity greater than n.
     * This function has no effect on the string length and cannot alter its content.
     */
    SharedBuffer* Reserve(
        /* [in] */ size_t newCapacity) const;

    inline size_t GetCapacity() const;

    //! acquire a reference on this buffer
    int32_t AddRef() const;

    /*! release a reference on this buffer, with the option of not
     * freeing the memory associated with it if it was the last reference
     * returns the previous reference count
     */
    int32_t Release(
        /* [in] */ uint32_t flags = 0) const;

    //! returns wether or not we're the only owner
    inline bool OnlyOwner() const;

private:
    inline SharedBuffer() {}
    inline ~SharedBuffer() {}
    SharedBuffer(const SharedBuffer&);
    SharedBuffer& operator= (const SharedBuffer&);

private:
    // Must be sized to preserve correct alignment.
    mutable std::atomic<int32_t> mRefs;
    size_t mSize;
    /* sizeof these fields (mReserved, mClientMetadata) == sizeof mCapacity
    uint32_t                    mReserved;
    // mClientMetadata is reserved for client use.  It is initialized to 0
    // and the clients can do whatever they want with it.  Note that this is
    // placed last so that it is adjcent to the buffer allocated.
    uint32_t                    mClientMetadata;
    */
    size_t mCapacity;

    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(short id, const void *p);

public:
    static CMemPoolSet *memPoolSet;
};

static_assert(sizeof(SharedBuffer) % 8 == 0
        && (sizeof(size_t) > 4 || sizeof(SharedBuffer) == 16),
        "SharedBuffer has unexpected size");
// ---------------------------------------------------------------------------

const void* SharedBuffer::GetData() const
{
    return this + 1;
}

void* SharedBuffer::GetData()
{
    return this + 1;
}

size_t SharedBuffer::GetSize() const
{
    return mSize;
}

size_t SharedBuffer::SetSize(size_t size)
{
    if ((size > 0) && (size <= mCapacity)) {
        mSize = size;
        return mSize;
    }
    return 0;
}

size_t SharedBuffer::GetCapacity() const
{
    return mCapacity;
}

SharedBuffer* SharedBuffer::GetBufferFromData(
    /* [in] */ void* data)
{
    if (data != nullptr) {
        return static_cast<SharedBuffer *>(data) - 1;
    }

    return 0;
}

const SharedBuffer* SharedBuffer::GetBufferFromData(
    /* [in] */ const void* data)
{
    if (data !=  nullptr) {
        return static_cast<const SharedBuffer *>(data) - 1;
    }

    return 0;
}

size_t SharedBuffer::GetSizeFromData(
    /* [in] */ const void* data)
{
    if (data != nullptr) {
        return GetBufferFromData(data)->mSize;
    }

    return 0;
}

bool SharedBuffer::OnlyOwner() const
{
    return (mRefs.load(std::memory_order_acquire) == 1);
}

} // namespace como

#endif // __COMO_SHAREDBUFFER_H__
