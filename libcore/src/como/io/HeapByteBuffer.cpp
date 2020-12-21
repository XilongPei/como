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

#include "como/io/Bits.h"
#include "como/io/ByteBufferAsCharBuffer.h"
#include "como/io/ByteBufferAsDoubleBuffer.h"
#include "como/io/ByteBufferAsFloatBuffer.h"
#include "como/io/ByteBufferAsIntegerBuffer.h"
#include "como/io/ByteBufferAsLongBuffer.h"
#include "como/io/ByteBufferAsShortBuffer.h"
#include "como/io/HeapByteBuffer.h"
#include "libcore/io/Memory.h"

using libcore::io::Memory;

namespace como {
namespace io {

ECode HeapByteBuffer::Constructor(
    /* [in] */ Integer cap,
    /* [in] */ Integer lim)
{
    return Constructor(cap, lim, false);
}

ECode HeapByteBuffer::Constructor(
    /* [in] */ Integer cap,
    /* [in] */ Integer lim,
    /* [in] */ Boolean isReadOnly)
{
    Array<Byte> buf(cap);
    FAIL_RETURN(ByteBuffer::Constructor(-1, 0, lim, cap, buf, 0));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapByteBuffer::Constructor(
    /* [in] */ const Array<Byte>& buf,
    /* [in] */ Integer off,
    /* [in] */ Integer len)
{
    return Constructor(buf, off, len, false);
}

ECode HeapByteBuffer::Constructor(
    /* [in] */ const Array<Byte>& buf,
    /* [in] */ Integer off,
    /* [in] */ Integer len,
    /* [in] */ Boolean isReadOnly)
{
    FAIL_RETURN(ByteBuffer::Constructor(-1, off, off + len, buf.GetLength(), buf, 0));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapByteBuffer::Constructor(
    /* [in] */ const Array<Byte>& buf,
    /* [in] */ Integer mark,
    /* [in] */ Integer pos,
    /* [in] */ Integer lim,
    /* [in] */ Integer cap,
    /* [in] */ Integer off,
    /* [in] */ Boolean isReadOnly)
{
    FAIL_RETURN(ByteBuffer::Constructor(mark, pos, lim, cap, buf, off));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapByteBuffer::Slice(
    /* [out] */ AutoPtr<IByteBuffer>& buffer)
{
    Integer remaining, pos;
    Remaining(remaining);
    GetPosition(pos);
    AutoPtr<HeapByteBuffer> hbb = new HeapByteBuffer();
    FAIL_RETURN(hbb->Constructor(
            mHb, -1, 0, remaining, remaining, pos + mOffset, mIsReadOnly));
    buffer = (IByteBuffer*)hbb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::Duplicate(
    /* [out] */ AutoPtr<IByteBuffer>& buffer)
{
    Integer pos, lim, cap;
    GetPosition(pos);
    GetLimit(lim);
    GetCapacity(cap);
    AutoPtr<HeapByteBuffer> hbb = new HeapByteBuffer();
    FAIL_RETURN(hbb->Constructor(
            mHb, MarkValue(), pos, lim, cap, mOffset, mIsReadOnly));
    buffer = (IByteBuffer*)hbb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::AsReadOnlyBuffer(
    /* [out] */ AutoPtr<IByteBuffer>& buffer)
{
    Integer pos, lim, cap;
    GetPosition(pos);
    GetLimit(lim);
    GetCapacity(cap);
    AutoPtr<HeapByteBuffer> hbb = new HeapByteBuffer();
    FAIL_RETURN(hbb->Constructor(
            mHb, MarkValue(), pos, lim, cap, mOffset, true));
    buffer = (IByteBuffer*)hbb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::Get(
    /* [out] */ Byte& b)
{
    Integer index;
    NextGetIndex(&index);
    b = mHb[Ix(index)];
    return NOERROR;
}

ECode HeapByteBuffer::Get(
    /* [in] */ Integer index,
    /* [out] */ Byte& b)
{
    FAIL_RETURN(CheckIndex(index));
    b = mHb[Ix(index)];
    return NOERROR;
}

ECode HeapByteBuffer::Get(
    /* [out] */ Array<Byte>& dst,
    /* [in] */ Integer offset,
    /* [in] */ Integer length)
{
    FAIL_RETURN(CheckBounds(mOffset, length, dst.GetLength()));
    Integer remaining;
    Remaining(remaining);
    if (length > remaining){
        return E_BUFFER_UNDERFLOW_EXCEPTION;
    }
    Integer pos;
    GetPosition(pos);
    dst.Copy(offset, mHb, Ix(pos), length);
    SetPosition(pos + length);
    return NOERROR;
}

ECode HeapByteBuffer::IsDirect(
    /* [out] */ Boolean& direct)
{
    direct = false;
    return NOERROR;
}

ECode HeapByteBuffer::IsReadOnly(
    /* [out] */ Boolean& readOnly)
{
    readOnly = mIsReadOnly;
    return NOERROR;
}

ECode HeapByteBuffer::Put(
    /* [in] */ Byte b)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(&index);
    mHb[Ix(index)] = b;
    return NOERROR;
}

ECode HeapByteBuffer::Put(
    /* [in] */ Integer index,
    /* [in] */ Byte b)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index));
    mHb[Ix(index)] = b;
    return NOERROR;
}

ECode HeapByteBuffer::Put(
    /* [in] */ const Array<Byte>& src,
    /* [in] */ Integer offset,
    /* [in] */ Integer length)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckBounds(offset, length, src.GetLength()));
    Integer remaining;
    Remaining(remaining);
    if (length > remaining) {
        return E_BUFFER_OVERFLOW_EXCEPTION;
    }
    Integer pos;
    GetPosition(pos);
    mHb.Copy(Ix(pos), src, offset, length);
    SetPosition(pos + length);
    return NOERROR;
}

ECode HeapByteBuffer::Compact()
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer pos, remaining, cap;
    GetPosition(pos);
    Remaining(remaining);
    mHb.Copy(Ix(0), mHb, Ix(pos), remaining);
    SetPosition(remaining);
    GetCapacity(cap);
    SetLimit(cap);
    DiscardMark();
    return NOERROR;
}

Byte HeapByteBuffer::_Get(
    /* [in] */ Integer i)
{
    return mHb[i];
}

ECode HeapByteBuffer::_Put(
    /* [in] */ Integer i,
    /* [in] */ Byte b)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    mHb[i] = b;
    return NOERROR;
}

ECode HeapByteBuffer::GetChar(
    /* [out] */ Char& c)
{
    Integer index;
    NextGetIndex(4, &index);
    c = Bits::GetChar(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetChar(
    /* [in] */ Integer index,
    /* [out] */ Char& c)
{
    FAIL_RETURN(CheckIndex(index, 4));
    c = Bits::GetChar(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetCharUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Char& c)
{
    c = Bits::GetChar(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Char>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 4, mHb, Ix(pos), 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutChar(
    /* [in] */ Char value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(4, &index);
    Bits::PutChar(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutChar(
    /* [in] */ Integer index,
    /* [in] */ Char value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 4));
    Bits::PutChar(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutCharUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Char value)
{
    Bits::PutChar(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Char>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 4, src, srcOffset, 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsCharBuffer(
    /* [out] */ AutoPtr<ICharBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 2;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsCharBuffer> bb = new ByteBufferAsCharBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (ICharBuffer*)bb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::GetShort(
    /* [out] */ Short& s)
{
    Integer index;
    NextGetIndex(2, &index);
    s = Bits::GetShort(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetShort(
    /* [in] */ Integer index,
    /* [out] */ Short& s)
{
    FAIL_RETURN(CheckIndex(index, 2));
    s = Bits::GetShort(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetShortUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Short& value)
{
    value = Bits::GetShort(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Short>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 2, mHb, Ix(pos), 2, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutShort(
    /* [in] */ Short value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(2, &index);
    Bits::PutShort(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutShort(
    /* [in] */ Integer index,
    /* [in] */ Short value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 2));
    Bits::PutShort(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutShortUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Short value)
{
    Bits::PutShort(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Short>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 2, src, srcOffset, 2, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsShortBuffer(
    /* [out] */ AutoPtr<IShortBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 1;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsShortBuffer> bb = new ByteBufferAsShortBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (IShortBuffer*)bb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::GetInteger(
    /* [out] */ Integer& i)
{
    Integer index;
    NextGetIndex(4, &index);
    i = Bits::GetInteger(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetInteger(
    /* [in] */ Integer index,
    /* [out] */ Integer& i)
{
    FAIL_RETURN(CheckIndex(index, 4));
    i = Bits::GetInteger(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetIntegerUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Integer& value)
{
    value = Bits::GetInteger(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Integer>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 4, mHb, Ix(pos), 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutInteger(
    /* [in] */ Integer value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(4, &index);
    Bits::PutInteger(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutInteger(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 4));
    Bits::PutInteger(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutIntegerUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    Bits::PutInteger(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Integer>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 4, src, srcOffset, 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsIntegerBuffer(
    /* [out] */ AutoPtr<IIntegerBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 2;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsIntegerBuffer> bb = new ByteBufferAsIntegerBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (IIntegerBuffer*)bb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::GetLong(
    /* [out] */ Long& l)
{
    Integer index;
    NextGetIndex(8, &index);
    l = Bits::GetLong(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetLong(
    /* [in] */ Integer index,
    /* [out] */ Long& l)
{
    FAIL_RETURN(CheckIndex(index, 8));
    l = Bits::GetLong(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetLongUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Long& value)
{
    value = Bits::GetLong(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Long>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 8, mHb, Ix(pos), 8, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutLong(
    /* [in] */ Long value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(8, &index);
    Bits::PutLong(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutLong(
    /* [in] */ Integer index,
    /* [in] */ Long value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 8));
    Bits::PutLong(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutLongUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Long value)
{
    Bits::PutLong(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Long>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 8, src, srcOffset, 8, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsLongBuffer(
    /* [out] */ AutoPtr<ILongBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 3;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsLongBuffer> bb = new ByteBufferAsLongBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (ILongBuffer*)bb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::GetFloat(
    /* [out] */ Float& f)
{
    Integer index;
    NextGetIndex(4, &index);
    f = Bits::GetFloat(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetFloat(
    /* [in] */ Integer index,
    /* [out] */ Float& f)
{
    FAIL_RETURN(CheckIndex(index, 4));
    f = Bits::GetFloat(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetFloatUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Float& f)
{
    f = Bits::GetFloat(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Float>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 4, mHb, Ix(pos), 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutFloat(
    /* [in] */ Float value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(4, &index);
    Bits::PutFloat(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutFloat(
    /* [in] */ Integer index,
    /* [in] */ Float value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 4));
    Bits::PutFloat(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutFloatUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Float value)
{
    Bits::PutFloat(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Float>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 4, src, srcOffset, 4, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsFloatBuffer(
    /* [out] */ AutoPtr<IFloatBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 2;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsFloatBuffer> bb = new ByteBufferAsFloatBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (IFloatBuffer*)bb.Get();
    return NOERROR;
}

ECode HeapByteBuffer::GetDouble(
    /* [out] */ Double& d)
{
    Integer index;
    NextGetIndex(8, &index);
    d = Bits::GetDouble(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetDouble(
    /* [in] */ Integer index,
    /* [out] */ Double& d)
{
    FAIL_RETURN(CheckIndex(index, 8));
    d = Bits::GetDouble(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetDoubleUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Double& value)
{
    value = Bits::GetDouble(this, Ix(index), mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::GetUnchecked(
    /* [in] */ Integer pos,
    /* [out] */ Array<Double>& dst,
    /* [in] */ Integer dstOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkGet(dst, dstOffset, length * 8, mHb, Ix(pos), 8, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::PutDouble(
    /* [in] */ Double value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(8, &index);
    Bits::PutDouble(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutDouble(
    /* [in] */ Integer index,
    /* [in] */ Double value)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index, 8));
    Bits::PutDouble(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutDoubleUnchecked(
    /* [in] */ Integer index,
    /* [in] */ Double value)
{
    Bits::PutDouble(this, Ix(index), value, mBigEndian);
    return NOERROR;
}

ECode HeapByteBuffer::PutUnchecked(
    /* [in] */ Integer pos,
    /* [in] */ const Array<Double>& src,
    /* [in] */ Integer srcOffset,
    /* [in] */ Integer length)
{
    Memory::UnsafeBulkPut(mHb, Ix(pos), length * 8, src, srcOffset, 8, !mNativeByteOrder);
    return NOERROR;
}

ECode HeapByteBuffer::AsDoubleBuffer(
    /* [out] */ AutoPtr<IDoubleBuffer>& buffer)
{
    Integer remaining, off;
    Remaining(remaining);
    Integer size = remaining >> 3;
    GetPosition(off);
    AutoPtr<IByteOrder> order;
    GetOrder(order);

    AutoPtr<ByteBufferAsDoubleBuffer> bb = new ByteBufferAsDoubleBuffer();
    FAIL_RETURN(bb->Constructor(this, -1, 0, size, size, off, order));
    buffer = (IDoubleBuffer*)bb.Get();
    return NOERROR;
}

}
}
