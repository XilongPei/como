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

#ifndef __COMO_IO_CHARSET_CHARSETDECODER_H__
#define __COMO_IO_CHARSET_CHARSETDECODER_H__

#include "como/core/SyncObject.h"
#include "como/io/charset/CodingErrorAction.h"
#include "como.io.IByteBuffer.h"
#include "como.io.ICharBuffer.h"
#include "como.io.charset.ICharset.h"
#include "como.io.charset.ICharsetDecoder.h"
#include "como.io.charset.ICodingErrorAction.h"
#include "como.io.charset.ICoderResult.h"

using como::core::SyncObject;

namespace como {
namespace io {
namespace charset {

class CharsetDecoder
    : public SyncObject
    , public ICharsetDecoder
{
public:
    COMO_INTERFACE_DECL();

    ECode GetCharset(
        /* [out] */ AutoPtr<ICharset>& cs) override;

    ECode GetReplacement(
        /* [out] */ String& replacement) override;

    ECode ReplaceWith(
        /* [in] */ const String& newReplacement) override;

    ECode GetMalformedInputAction(
        /* [out] */ AutoPtr<ICodingErrorAction>& action) override;

    ECode OnMalformedInput(
        /* [in] */ ICodingErrorAction* newAction) override;

    ECode GetUnmappableCharacterAction(
        /* [out] */ AutoPtr<ICodingErrorAction>& action) override;

    ECode OnUnmappableCharacter(
        /* [in] */ ICodingErrorAction* newAction) override;

    ECode GetAverageCharsPerByte(
        /* [out] */ Float& averageCharsPerByte) override;

    ECode GetMaxCharsPerByte(
        /* [out] */ Float& maxCharsPerByte) override;

    ECode Decode(
        /* [in] */ IByteBuffer* bb,
        /* [out] */ ICharBuffer* cb,
        /* [in] */ Boolean endOfInput,
        /* [out] */ AutoPtr<ICoderResult>& result) override;

    ECode Flush(
        /* [out] */ ICharBuffer* cb,
        /* [out] */ AutoPtr<ICoderResult>& result) override;

    ECode Reset() override;

    ECode Decode(
        /* [in] */ IByteBuffer* bb,
        /* [out] */ AutoPtr<ICharBuffer>& cb) override;

    ECode IsAutoDetecting(
        /* [out] */ Boolean& autoDetecting) override;

    ECode IsCharsetDetected(
        /* [out] */ Boolean& charsetDetected) override;

    ECode GetDetectedCharset(
        /* [out] */ AutoPtr<ICharset>& cs) override;

protected:
    ECode Constructor(
        /* [in] */ ICharset* cs,
        /* [in] */ Float averageCharsPerByte,
        /* [in] */ Float maxCharsPerByte);

    virtual void ImplReplaceWith(
        /* [in] */ const String& newReplacement)
    {}

    virtual void ImplOnMalformedInput(
        /* [in] */ ICodingErrorAction* newAction)
    {}

    virtual void ImplOnUnmappableCharacter(
        /* [in] */ ICodingErrorAction* newAction)
    {}

    virtual ECode ImplFlush(
        /* [out] */ ICharBuffer* cb,
        /* [out] */ AutoPtr<ICoderResult>& result);

    virtual void ImplReset()
    {}

    virtual ECode DecodeLoop(
        /* [in] */ IByteBuffer* bb,
        /* [out] */ ICharBuffer* cb,
        /* [out] */ AutoPtr<ICoderResult>& result) = 0;

private:
    ECode Constructor(
        /* [in] */ ICharset* cs,
        /* [in] */ Float averageCharsPerByte,
        /* [in] */ Float maxCharsPerByte,
        /* [in] */ const String& replacement);

private:
    AutoPtr<ICharset> mCharset;
    Float mAverageCharsPerByte = 0;
    Float mMaxCharsPerByte = 0;

    String mReplacement;
    AutoPtr<ICodingErrorAction> mMalformedInputAction =
            CodingErrorAction::GetREPORT();
    AutoPtr<ICodingErrorAction> mUnmappableCharacterAction =
            CodingErrorAction::GetREPORT();

    static constexpr Integer ST_RESET = 0;
    static constexpr Integer ST_CODING = 1;
    static constexpr Integer ST_END = 2;
    static constexpr Integer ST_FLUSHED = 3;

    Integer mState = ST_RESET;
};

}
}
}

#endif // __COMO_IO_CHARSET_CHARSETDECODER_H__
