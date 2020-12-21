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

#ifndef __COMO_UTIL_LOCALE_BASELOCALE_H__
#define __COMO_UTIL_LOCALE_BASELOCALE_H__

#include "como/core/volatile.h"
#include "como/util/locale/LocaleObjectCache.h"
#include "como.core.IComparable.h"
#include <comosp.h>
#include <comoobj.h>

using como::core::IComparable;

namespace como {
namespace util {
namespace locale {

class BaseLocale final
    : public Object
{
private:
    class Key
        : public Object
        , public IComparable
    {
    public:
        COMO_INTERFACE_DECL();

        Key(
            /* [in] */ const String& language,
            /* [in] */ const String& region);

        inline Key(
            /* [in] */ const String& language,
            /* [in] */ const String& script,
            /* [in] */ const String& region,
            /* [in] */ const String& variant)
            : Key(language, script, region, variant, false)
        {}

        Key(
            /* [in] */ const String& language,
            /* [in] */ const String& script,
            /* [in] */ const String& region,
            /* [in] */ const String& variant,
            /* [in] */ Boolean normalized);

        ECode Equals(
            /* [in] */ IInterface* obj,
            /* [out] */ Boolean& same) override;

        ECode CompareTo(
            /* [in] */ IInterface* obj,
            /* [out] */ Integer& result) override;

        ECode GetHashCode(
            /* [out] */ Integer& hash) override;

        static AutoPtr<Key> Normalize(
            /* [in] */ Key* key);

    public:
        String mLang;
        String mScrt;
        String mRegn;
        String mVart;
        Boolean mNormalized;
        Integer mHash;
    };

    class Cache
        : public LocaleObjectCache
    {
    public:
        AutoPtr<IInterface> NormalizeKey(
            /* [in] */ IInterface* key) override;

        AutoPtr<IInterface> CreateObject(
            /* [in] */ IInterface* key) override;
    };

public:
    IInterface* Probe(
        /* [in] */ const InterfaceID& iid) override;

    static AutoPtr<BaseLocale> CreateInstance(
        /* [in] */ const String& language,
        /* [in] */ const String& region);

    static AutoPtr<BaseLocale> GetInstance(
        /* [in] */ const String& language,
        /* [in] */ const String& script,
        /* [in] */ const String& region,
        /* [in] */ const String& variant);

    String GetLanguage();

    String GetScript();

    String GetRegion();

    String GetVariant();

    ECode Equals(
        /* [in] */ IInterface* obj,
        /* [out] */ Boolean& same) override;

    ECode ToString(
        /* [out] */ String& desc) override;

    ECode GetHashCode(
        /* [out] */ Integer& hash) override;

private:
    static Cache* Get_CACHE();

    BaseLocale(
        /* [in] */ const String& language,
        /* [in] */ const String& region);

    BaseLocale(
        /* [in] */ const String& language,
        /* [in] */ const String& script,
        /* [in] */ const String& region,
        /* [in] */ const String& variant);

public:
    static const String SEP;

private:
    String mLanguage;
    String mScript;
    String mRegion;
    String mVariant;

    VOLATILE Integer mHash = 0;
};

inline String BaseLocale::GetLanguage()
{
    return mLanguage;
}

inline String BaseLocale::GetScript()
{
    return mScript;
}

inline String BaseLocale::GetRegion()
{
    return mRegion;
}

inline String BaseLocale::GetVariant()
{
    return mVariant;
}

}
}
}

#endif // __COMO_UTIL_LOCALE_BASELOCALE_H__
