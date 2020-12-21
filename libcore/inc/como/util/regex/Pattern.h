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

#ifndef __COMO_UTIL_REGEX_PATTERN_H__
#define __COMO_UTIL_REGEX_PATTERN_H__

#include "como.core.ICharSequence.h"
#include "como.io.ISerializable.h"
#include "como.util.regex.IMatcher.h"
#include "como.util.regex.IPattern.h"
#include "como/core/SyncObject.h"

using como::core::ICharSequence;
using como::core::SyncObject;
using como::io::ISerializable;

namespace como {
namespace util {
namespace regex {

class Pattern final
    : public SyncObject
    , public IPattern
    , public ISerializable
{
public:
    COMO_INTERFACE_DECL();

    ~Pattern();

    static ECode Compile(
        /* [in] */ const String& regex,
        /* [out] */ AutoPtr<IPattern>& pattern);

    static ECode Compile(
        /* [in] */ const String& regex,
        /* [in] */ Integer flags,
        /* [out] */ AutoPtr<IPattern>& pattern);

    ECode GetPattern(
        /* [out] */ String& pattStr) override;

    ECode ToString(
        /* [out] */ String& pattStr) override;

    ECode Matcher(
        /* [in] */ ICharSequence* input,
        /* [out] */ AutoPtr<IMatcher>& matcher) override;

    ECode Flags(
        /* [out] */ Integer& flags) override;

    static ECode Matches(
        /* [in] */ const String& regex,
        /* [in] */ ICharSequence* input,
        /* [out] */ Boolean& matched);

    ECode Split(
        /* [in] */ ICharSequence* input,
        /* [in] */ Integer limit,
        /* [out, callee] */ Array<String>* strArray) override;

    static ECode FastSplit(
        /* [in] */ const String& re,
        /* [in] */ const String& input,
        /* [in] */ Integer limit,
        /* [out, callee] */ Array<String>* strArray);

    ECode Split(
        /* [in] */ ICharSequence* input,
        /* [out, callee] */ Array<String>* strArray) override;

    static ECode Quote(
        /* [in] */ const String& s,
        /* [out] */ String& pattStr);

    static Pattern* From(
        /* [in] */ IPattern* p);

private:
    Pattern();

    ECode Constructor(
        /* [in] */ const String& p,
        /* [in] */ Integer f);

    ECode Compile();

    static ECode CompileImpl(
        /* [in] */ const String& regex,
        /* [in] */ Integer flags,
        /* [out] */ HANDLE& handle);

private:
    friend class Matcher;

    String mPattern;

    Integer mFlags = 0;

    HANDLE mNative = 0;
};

inline Pattern* Pattern::From(
    /* [in] */ IPattern* p)
{
    return (Pattern*)p;
}

}
}
}

#endif // __COMO_UTIL_REGEX_PATTERN_H__
