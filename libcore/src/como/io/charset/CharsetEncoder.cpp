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

#include "como/io/ByteBuffer.h"
#include "como/io/CharBuffer.h"
#include "como/io/charset/CharsetEncoder.h"
#include "como/io/charset/CoderResult.h"
#include "como/util/Arrays.h"
#include "como.io.IBuffer.h"
#include "como.io.charset.ICharsetDecoder.h"

using como::core::E_ILLEGAL_STATE_EXCEPTION;
using como::util::Arrays;

namespace como {
namespace io {
namespace charset {

COMO_INTERFACE_IMPL_1(CharsetEncoder, SyncObject, ICharsetEncoder);

ECode CharsetEncoder::Constructor(
    /* [in] */ ICharset* cs,
    /* [in] */ Float averageBytesPerChar,
    /* [in] */ Float maxBytesPerChar,
    /* [in] */ const Array<Byte>& replacement)
{
    return Constructor(cs, averageBytesPerChar, maxBytesPerChar, replacement, false);
}

ECode CharsetEncoder::Constructor(
    /* [in] */ ICharset* cs,
    /* [in] */ Float averageBytesPerChar,
    /* [in] */ Float maxBytesPerChar,
    /* [in] */ const Array<Byte>& replacement,
    /* [in] */ Boolean trusted)
{
    mCharset = cs;
    if (averageBytesPerChar <= 0.0f) {
        Logger::E("CharsetDecoder", "Non-positive averageBytesPerChar");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if (maxBytesPerChar <= 0.0f) {
        Logger::E("CharsetDecoder", "Non-positive maxBytesPerChar");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mReplacement = replacement;
    mAverageBytesPerChar = averageBytesPerChar;
    mMaxBytesPerChar = maxBytesPerChar;
    if (!trusted) {
        FAIL_RETURN(ReplaceWith(mReplacement));
    }
    return NOERROR;
}

ECode CharsetEncoder::Constructor(
    /* [in] */ ICharset* cs,
    /* [in] */ Float averageBytesPerChar,
    /* [in] */ Float maxBytesPerChar)
{
    Array<Byte> replacement{ '?' };
    return Constructor(cs, averageBytesPerChar, maxBytesPerChar, replacement);
}

ECode CharsetEncoder::GetCharset(
    /* [out] */ AutoPtr<ICharset>& cs)
{
    cs = mCharset;
    return NOERROR;
}

ECode CharsetEncoder::GetReplacement(
    /* [out, callee] */ Array<Byte>* replacement)
{
    VALIDATE_NOT_NULL(replacement);

    return Arrays::CopyOf(mReplacement, mReplacement.GetLength(), replacement);
}

ECode CharsetEncoder::ReplaceWith(
    /* [in] */ const Array<Byte>& newReplacement)
{
    if (newReplacement.IsNull()) {
        Logger::E("CharsetEncoder", "Null replacement");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Integer len = newReplacement.GetLength();
    if (len == 0) {
        Logger::E("CharsetEncoder", "Empty replacement");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (len > mMaxBytesPerChar) {
        Logger::E("CharsetEncoder", "Replacement too long");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    Boolean legal;
    if (IsLegalReplacement(newReplacement, legal), !legal) {
        Logger::E("CharsetEncoder", "Illegal replacement");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Arrays::CopyOf(newReplacement, newReplacement.GetLength(), &mReplacement);
    ImplReplaceWith(mReplacement);
    return NOERROR;
}

ECode CharsetEncoder::IsLegalReplacement(
    /* [in] */ const Array<Byte>& repl,
    /* [out] */ Boolean& isLegal)
{
    AutoPtr<IWeakReference> wr = mCachedDecoder;
    AutoPtr<ICharsetDecoder> dec;
    if (wr != nullptr) {
        wr->Resolve(IID_ICharsetDecoder, (IInterface**)&dec);
    }
    if (dec == nullptr) {
        AutoPtr<ICharset> cs;
        GetCharset(cs);
        cs->NewDecoder(dec);
        dec->OnMalformedInput(CodingErrorAction::GetREPORT());
        dec->OnUnmappableCharacter(CodingErrorAction::GetREPORT());
        IWeakReferenceSource::Probe(dec)->GetWeakReference(mCachedDecoder);
    }
    else {
        dec->Reset();
    }
    AutoPtr<IByteBuffer> bb;
    ByteBuffer::Wrap(repl, &bb);
    Integer remaining;
    Float maxCharsPerByte;
    IBuffer::Probe(bb)->Remaining(remaining);
    dec->GetMaxCharsPerByte(maxCharsPerByte);
    AutoPtr<ICharBuffer> cb;
    CharBuffer::Allocate(remaining * maxCharsPerByte, &cb);
    AutoPtr<ICoderResult> cr;
    dec->Decode(bb, cb, true, cr);
    Boolean error;
    cr->IsError(error);
    isLegal = !error;
    return NOERROR;
}

ECode CharsetEncoder::GetMalformedInputAction(
    /* [out] */ AutoPtr<ICodingErrorAction>& action)
{
    action = mMalformedInputAction;
    return NOERROR;
}

ECode CharsetEncoder::OnMalformedInput(
    /* [in] */ ICodingErrorAction* newAction)
{
    if (newAction == nullptr) {
        Logger::E("CharsetDecoder", "Null action");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mMalformedInputAction = newAction;
    ImplOnMalformedInput(newAction);
    return NOERROR;
}

ECode CharsetEncoder::GetUnmappableCharacterAction(
    /* [out] */ AutoPtr<ICodingErrorAction>& action)
{
    action = mUnmappableCharacterAction;
    return NOERROR;
}

ECode CharsetEncoder::OnUnmappableCharacter(
    /* [in] */ ICodingErrorAction* newAction)
{
    if (newAction == nullptr) {
        Logger::E("CharsetDecoder", "Null action");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mUnmappableCharacterAction = newAction;
    ImplOnUnmappableCharacter(newAction);
    return NOERROR;
}

ECode CharsetEncoder::GetAverageBytesPerChar(
    /* [out] */ Float& averageBytesPerChar)
{
    averageBytesPerChar = mAverageBytesPerChar;
    return NOERROR;
}

ECode CharsetEncoder::GetMaxBytesPerChar(
    /* [out] */ Float& maxBytesPerChar)
{
    maxBytesPerChar = mMaxBytesPerChar;
    return NOERROR;
}

ECode CharsetEncoder::Encode(
    /* [in] */ ICharBuffer* cb,
    /* [out] */ IByteBuffer* bb,
    /* [in] */ Boolean endOfInput,
    /* [out] */ AutoPtr<ICoderResult>& result)
{
    Integer newState = endOfInput ? ST_END : ST_CODING;
    if ((mState != ST_RESET) && (mState != ST_CODING) &&
            !(endOfInput && (mState == ST_END))) {
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    mState = newState;

    for (;;) {
        ECode ec = EncodeLoop(cb, bb, result);
        if (FAILED(ec)) {
            return E_CODER_MALFUNCTION_ERROR;
        }

        Boolean overflow;
        if (result->IsOverflow(overflow), overflow) {
            return NOERROR;
        }

        Boolean underflow;
        if (result->IsUnderflow(underflow), underflow) {
            Boolean hasRemaining;
            if (endOfInput && (IBuffer::Probe(cb)->HasRemaining(hasRemaining), hasRemaining)) {
                Integer remaining;
                IBuffer::Probe(cb)->Remaining(remaining);
                result = CoderResult::MalformedForLength(remaining);
            }
            else {
                return NOERROR;
            }
        }

        AutoPtr<ICodingErrorAction> action;
        Boolean malformed, unmappable;
        if (result->IsMalformed(malformed), malformed) {
            action = mMalformedInputAction;
        }
        else if (result->IsUnmappable(unmappable), unmappable) {
            action = mUnmappableCharacterAction;
        }
        else {
            CHECK(false);
        }

        if (action == CodingErrorAction::GetREPORT()) {
            return NOERROR;
        }

        if (action == CodingErrorAction::GetREPLACE()) {
            Integer remaining;
            if (IBuffer::Probe(bb)->Remaining(remaining), remaining < mReplacement.GetLength()) {
                result = CoderResult::GetOVERFLOW();
                return NOERROR;
            }
            bb->Put(mReplacement);
        }

        if (action == CodingErrorAction::GetIGNORE() ||
                (action == CodingErrorAction::GetREPLACE())) {
            Integer pos, len;
            IBuffer::Probe(cb)->GetPosition(pos);
            result->GetLength(len);
            IBuffer::Probe(cb)->SetPosition(pos + len);
            continue;
        }

        CHECK(false);
    }
}

ECode CharsetEncoder::Flush(
    /* [out] */ IByteBuffer* bb,
    /* [out] */ AutoPtr<ICoderResult>& result)
{
    if (mState == ST_END) {
        FAIL_RETURN(ImplFlush(bb, result));
        Boolean underflow;
        if (result->IsUnderflow(underflow), underflow) {
            mState = ST_FLUSHED;
        }
        return NOERROR;
    }

    if (mState != ST_FLUSHED) {
        return E_ILLEGAL_STATE_EXCEPTION;
    }

    result = CoderResult::GetUNDERFLOW();
    return NOERROR;
}

ECode CharsetEncoder::ImplFlush(
    /* [out] */ IByteBuffer* bb,
    /* [out] */ AutoPtr<ICoderResult>& cr)
{
    cr = CoderResult::GetUNDERFLOW();
    return NOERROR;
}

ECode CharsetEncoder::Reset()
{
    ImplReset();
    mState = ST_RESET;
    return NOERROR;
}

ECode CharsetEncoder::Encode(
    /* [in] */ ICharBuffer* cb,
    /* [out] */ AutoPtr<IByteBuffer>& bb)
{
    Integer remaining;
    Float averageBytesPerChar;
    IBuffer::Probe(cb)->Remaining(remaining);
    GetAverageBytesPerChar(averageBytesPerChar);
    Integer n = remaining * averageBytesPerChar;
    ByteBuffer::Allocate(n, &bb);

    if ((n == 0) && (remaining == 0)) {
        return NOERROR;
    }
    Reset();
    for (;;) {
        AutoPtr<ICoderResult> cr;
        Boolean result;
        if (IBuffer::Probe(cb)->HasRemaining(result), result) {
            FAIL_RETURN(Encode(cb, bb, true, cr));
        }
        else {
            cr = CoderResult::GetUNDERFLOW();
        }
        if (cr->IsUnderflow(result), result) {
            Flush(bb, cr);
        }

        if (cr->IsUnderflow(result), result) {
            break;
        }
        if (cr->IsOverflow(result), result) {
            n = 2 * n + 1;
            AutoPtr<IByteBuffer> o;
            ByteBuffer::Allocate(n, &o);
            IBuffer::Probe(bb)->Flip();
            o->Put(bb);
            bb = o;
            continue;
        }
        return cr->ThrowException();
    }
    IBuffer::Probe(bb)->Flip();
    return NOERROR;
}

Boolean CharsetEncoder::CanEncode(
    /* [in] */ ICharBuffer* cb)
{
    Boolean hasRemaining;
    if (IBuffer::Probe(cb)->HasRemaining(hasRemaining), !hasRemaining) {
        return true;
    }

    if (mState == ST_FLUSHED) {
        Reset();
    }
    else if (mState != ST_RESET) {
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    AutoPtr<ICodingErrorAction> ma, ua;
    GetMalformedInputAction(ma);
    GetUnmappableCharacterAction(ua);
    OnMalformedInput(CodingErrorAction::GetREPORT());
    OnUnmappableCharacter(CodingErrorAction::GetREPORT());
    AutoPtr<IByteBuffer> buf;
    ECode ec = Encode(cb, buf);
    OnMalformedInput(ma);
    OnUnmappableCharacter(ua);
    Reset();
    if (FAILED(ec)) {
        return false;
    }
    IBuffer::Probe(buf)->HasRemaining(hasRemaining);
    return hasRemaining;
}

ECode CharsetEncoder::CanEncode(
    /* [in] */ Char c,
    /* [out] */ Boolean& result)
{
    AutoPtr<ICharBuffer> cb;
    CharBuffer::Allocate(1, &cb);
    cb->Put(c);
    IBuffer::Probe(cb)->Flip();
    result = CanEncode(cb);
    return NOERROR;
}

ECode CharsetEncoder::CanEncode(
    /* [in] */ ICharSequence* cs,
    /* [out] */ Boolean& result)
{
    AutoPtr<ICharBuffer> cb;
    if (ICharBuffer::Probe(cs) != nullptr) {
        ICharBuffer::Probe(cs)->Duplicate(cb);
    }
    else {
        CharBuffer::Wrap(cs, &cb);
    }
    result = CanEncode(cb);
    return NOERROR;
}

}
}
}
