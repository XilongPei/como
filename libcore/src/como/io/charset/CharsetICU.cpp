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

#include "como/io/charset/CharsetICU.h"
#include "como/io/charset/CharsetDecoderICU.h"
#include "como/io/charset/CharsetEncoderICU.h"
#include "libcore/icu/NativeConverter.h"

using libcore::icu::NativeConverter;

namespace como {
namespace io {
namespace charset {

ECode CharsetICU::Constructor(
    /* [in] */ const String& canonicalName,
    /* [in] */ const String& icuCanonName,
    /* [in] */ const Array<String>& aliases)
{
    FAIL_RETURN(Charset::Constructor(canonicalName, aliases));
    mIcuCanonicalName = icuCanonName;
    return NOERROR;
}

ECode CharsetICU::NewDecoder(
    /* [out] */ AutoPtr<ICharsetDecoder>& decoder)
{
    return CharsetDecoderICU::NewInstance(
            this, mIcuCanonicalName, &decoder);
}

ECode CharsetICU::NewEncoder(
    /* [out] */ AutoPtr<ICharsetEncoder>& encoder)
{
    return CharsetEncoderICU::NewInstance(
            this, mIcuCanonicalName, &encoder);
}

ECode CharsetICU::Contains(
    /* [in] */ ICharset* cs,
    /* [out] */ Boolean& contains)
{
    if (cs == nullptr) {
        contains = false;
        return NOERROR;
    }
    Boolean equal;
    Equals(cs, equal);
    if (equal) {
        contains = true;
        return NOERROR;
    }
    String thName, csName;
    GetName(thName);
    cs->GetName(csName);
    contains = NativeConverter::Contains(thName, csName);
    return NOERROR;
}

}
}
}
