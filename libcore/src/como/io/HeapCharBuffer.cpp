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

#include "como/io/ByteOrder.h"
#include "como/io/HeapCharBuffer.h"

using como::core::E_INDEX_OUT_OF_BOUNDS_EXCEPTION;

namespace como {
namespace io {

extern const CoclassID CID_HeapCharBuffer =
        {{0xcc2cd1e7,0x7c8b,0x4374,0xac26,{0xe0,0x78,0x69,0x12,0x0b,0xfd}}, &CID_libcore};

ECode HeapCharBuffer::Constructor(
    /* [in] */ Integer cap,
    /* [in] */ Integer lim)
{
    return Constructor(cap, lim, false);
}

ECode HeapCharBuffer::Constructor(
    /* [in] */ Integer cap,
    /* [in] */ Integer lim,
    /* [in] */ Boolean isReadOnly)
{
    Array<Char> hb(cap);
    FAIL_RETURN(CharBuffer::Constructor(-1, 0, lim, cap, hb, 0));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapCharBuffer::Constructor(
    /* [in] */ const Array<Char>& buf,
    /* [in] */ Integer off,
    /* [in] */ Integer len)
{
    return Constructor(buf, off, len, false);
}

ECode HeapCharBuffer::Constructor(
    /* [in] */ const Array<Char>& buf,
    /* [in] */ Integer off,
    /* [in] */ Integer len,
    /* [in] */ Boolean isReadOnly)
{
    FAIL_RETURN(CharBuffer::Constructor(-1, off, off + len, buf.GetLength(), buf, 0));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapCharBuffer::Constructor(
    /* [in] */ const Array<Char>& buf,
    /* [in] */ Integer mark,
    /* [in] */ Integer pos,
    /* [in] */ Integer lim,
    /* [in] */ Integer cap,
    /* [in] */ Integer off)
{
    return Constructor(buf, mark, pos, lim, cap, off, false);
}

ECode HeapCharBuffer::Constructor(
    /* [in] */ const Array<Char>& buf,
    /* [in] */ Integer mark,
    /* [in] */ Integer pos,
    /* [in] */ Integer lim,
    /* [in] */ Integer cap,
    /* [in] */ Integer off,
    /* [in] */ Boolean isReadOnly)
{
    FAIL_RETURN(CharBuffer::Constructor(mark, pos, lim, cap, buf, off));
    mIsReadOnly = isReadOnly;
    return NOERROR;
}

ECode HeapCharBuffer::Slice(
    /* [out] */ AutoPtr<ICharBuffer>& buffer)
{
    Integer remaining, pos;
    Remaining(remaining);
    GetPosition(pos);
    AutoPtr<HeapCharBuffer> hcb = new HeapCharBuffer();
    FAIL_RETURN(hcb->Constructor(
            mHb, -1, 0, remaining, remaining, pos + mOffset, mIsReadOnly));
    buffer = (ICharBuffer*)hcb.Get();
    return NOERROR;
}

ECode HeapCharBuffer::Duplicate(
    /* [out] */ AutoPtr<ICharBuffer>& buffer)
{
    Integer pos, lim, cap;
    GetPosition(pos);
    GetLimit(lim);
    GetCapacity(cap);
    AutoPtr<HeapCharBuffer> hcb = new HeapCharBuffer();
    FAIL_RETURN(hcb->Constructor(
            mHb, MarkValue(), pos, lim, cap, mOffset, mIsReadOnly));
    buffer = (ICharBuffer*)hcb.Get();
    return NOERROR;
}

ECode HeapCharBuffer::AsReadOnlyBuffer(
    /* [out] */ AutoPtr<ICharBuffer>& buffer)
{
    Integer pos, lim, cap;
    GetPosition(pos);
    GetLimit(lim);
    GetCapacity(cap);
    AutoPtr<HeapCharBuffer> hcb = new HeapCharBuffer();
    FAIL_RETURN(hcb->Constructor(
            mHb, MarkValue(), pos, lim, cap, mOffset, true));
    buffer = (ICharBuffer*)hcb.Get();
    return NOERROR;
}

ECode HeapCharBuffer::Get(
    /* [out] */ Char& c)
{
    Integer index;
    NextGetIndex(&index);
    c = mHb[Ix(index)];
    return NOERROR;
}

ECode HeapCharBuffer::Get(
    /* [in] */ Integer index,
    /* [out] */ Char& c)
{
    FAIL_RETURN(CheckIndex(index));
    c = mHb[Ix(index)];
    return NOERROR;
}

ECode HeapCharBuffer::GetUnchecked(
    /* [in] */ Integer index,
    /* [out] */ Char& c)
{
    c = mHb[Ix(index)];
    return NOERROR;
}

ECode HeapCharBuffer::Get(
    /* [out] */ Array<Char>& dst,
    /* [in] */ Integer offset,
    /* [in] */ Integer length)
{
    FAIL_RETURN(CheckBounds(offset, length, dst.GetLength()));
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

ECode HeapCharBuffer::IsDirect(
    /* [out] */ Boolean& direct)
{
    direct = false;
    return NOERROR;
}

ECode HeapCharBuffer::IsReadOnly(
    /* [out] */ Boolean& readOnly)
{
    readOnly = mIsReadOnly;
    return NOERROR;
}

ECode HeapCharBuffer::Put(
    /* [in] */ Char c)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Integer index;
    NextPutIndex(&index);
    mHb[Ix(index)] = c;
    return NOERROR;
}

ECode HeapCharBuffer::Put(
    /* [in] */ Integer index,
    /* [in] */ Char c)
{
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    FAIL_RETURN(CheckIndex(index));
    mHb[Ix(index)] = c;
    return NOERROR;
}

ECode HeapCharBuffer::Put(
    /* [in] */ const Array<Char>& src,
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

ECode HeapCharBuffer::Put(
    /* [in] */ ICharBuffer* src)
{
    if (src == (ICharBuffer*)this) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (mIsReadOnly) {
        return E_READ_ONLY_BUFFER_EXCEPTION;
    }
    Boolean direct;
    if (Object::InstanceOf(src, CID_HeapCharBuffer)) {
        HeapCharBuffer* hcb = (HeapCharBuffer*)src;
        Integer n, remaining;
        hcb->Remaining(n);
        if (Remaining(remaining), n > remaining) {
            return E_BUFFER_OVERFLOW_EXCEPTION;
        }
        Integer thisPos, hcbPos;
        GetPosition(thisPos);
        hcb->GetPosition(hcbPos);
        mHb.Copy(Ix(thisPos), hcb->mHb, hcb->Ix(hcbPos), n);
        hcb->SetPosition(hcbPos + n);
        SetPosition(thisPos + n);
        return NOERROR;
    }
    else if (IBuffer::Probe(src)->IsDirect(direct), direct) {
        Integer n, remaining;
        IBuffer::Probe(src)->Remaining(n);
        if (Remaining(remaining), n > remaining) {
            return E_BUFFER_OVERFLOW_EXCEPTION;
        }
        Integer pos;
        GetPosition(pos);
        src->Get(mHb, Ix(pos), n);
        SetPosition(pos + n);
        return NOERROR;
    }
    else {
        return CharBuffer::Put(src);
    }
}

ECode HeapCharBuffer::Compact()
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

ECode HeapCharBuffer::ToString(
    /* [in] */ Integer start,
    /* [in] */ Integer end,
    /* [out] */ String& desc)
{
    desc = String(mHb, start + mOffset, end - start);
    return NOERROR;
}

ECode HeapCharBuffer::SubSequence(
    /* [in] */ Integer start,
    /* [in] */ Integer end,
    /* [out] */ AutoPtr<ICharSequence>& subcsq)
{
    Integer len;
    if ((start < 0) ||
            (GetLength(len), end > len) ||
            (start > end)) {
        subcsq = nullptr;
        return E_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    }
    Integer pos, cap;
    GetPosition(pos);
    GetCapacity(cap);
    AutoPtr<HeapCharBuffer> hcb = new HeapCharBuffer();
    FAIL_RETURN(hcb->Constructor(
            mHb, -1, pos + start, pos + end, cap, mOffset, mIsReadOnly));
    subcsq = (ICharSequence*)hcb.Get();
    return NOERROR;
}

ECode HeapCharBuffer::GetOrder(
    /* [out] */ AutoPtr<IByteOrder>& bo)
{
    bo = ByteOrder::Order();
    return NOERROR;
}

ECode HeapCharBuffer::GetCoclassID(
    /* [out] */ CoclassID& cid)
{
    cid = CID_HeapCharBuffer;
    return NOERROR;
}

}
}

