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

#ifndef __CDLC_COCLASSTYPE_H__
#define __CDLC_COCLASSTYPE_H__

#include "ast/Attributes.h"
#include "ast/InterfaceType.h"
#include "ast/Method.h"
#include "ast/Type.h"
#include "util/AutoPtr.h"
#include "util/String.h"
#include "util/UUID.h"

namespace cdlc {

class CoclassType
    : public Type
{
public:
    bool IsCoclassType() override;

    void SetAttributes(
        /* [in] */ const Attributes& attrs);

    inline AutoPtr<UUID> GetUUID();

    inline String GetVersion();

    inline String GetFuncSafetySetting();

    inline void AddConstructor(
        /* [in] */ Method* constructor);

    AutoPtr<Method> FindConstructor(
        /* [in] */ const String& name,
        /* [in] */ const String& signature);

    AutoPtr<Method> GetConstructor(
        /* [in] */ int i);

    inline int GetConstructorNumber();

    inline bool HasDefaultConstructor();

    inline bool IsConstructorDeleted();

    inline void AddInterface(
        /* [in] */ InterfaceType* interface);

    inline void InsertInterface(
        /* [in] */ InterfaceType* interface);

    AutoPtr<InterfaceType> GetInterface(
        /* [in] */ int i);

    inline int GetInterfaceNumber();

    inline AutoPtr<UUID> GetInterfaceUUID(int pos);

    String GetSignature() override;

    inline static AutoPtr<CoclassType> CastFrom(
        /* [in] */ Type* type);

    inline void AddConstant(
        /* [in] */ Constant* constant);

    AutoPtr<Constant> FindConstant(
        /* [in] */ const String& name);

    AutoPtr<Constant> GetConstant(
        /* [in] */ int i);

    inline int GetConstantNumber();

    inline String GetStrFramacBlock();

private:
    AutoPtr<UUID> mUuid;
    String mVersion;
    String mDescription;
    String mFuncSafetySetting;
    String mStrFramacBlock;
    std::vector<AutoPtr<Method>> mConstructors;
    std::vector<AutoPtr<InterfaceType>> mInterfaces;
    std::vector<AutoPtr<Constant>> mConstants;
};

AutoPtr<UUID> CoclassType::GetUUID()
{
    return mUuid;
}

String CoclassType::GetVersion()
{
    return mVersion;
}

String CoclassType::GetFuncSafetySetting()
{
    return mFuncSafetySetting;
}

void CoclassType::AddConstructor(
    /* [in] */ Method* constructor)
{
    if (constructor != nullptr) {
        mConstructors.push_back(constructor);
    }
}

int CoclassType::GetConstructorNumber()
{
    return mConstructors.size();
}

bool CoclassType::HasDefaultConstructor()
{
    return GetConstructorNumber() == 0;
}

bool CoclassType::IsConstructorDeleted()
{
    if (GetConstructorNumber() > 0) {
        return GetConstructor(0)->IsDeleted();
    }
    return false;
}

void CoclassType::AddInterface(
    /* [in] */ InterfaceType *interface)
{
    if (interface != nullptr) {
        mInterfaces.push_back(interface);
    }
}

void CoclassType::InsertInterface(
    /* [in] */ InterfaceType *interface)
{
    if (interface != nullptr) {
        mInterfaces.insert(mInterfaces.begin(), interface);
    }
}

int CoclassType::GetInterfaceNumber()
{
    return mInterfaces.size();
}

AutoPtr<UUID> CoclassType::GetInterfaceUUID(int pos)
{
    return mInterfaces[pos]->GetUUID();
}

AutoPtr<CoclassType> CoclassType::CastFrom(
    /* [in] */ Type* type)
{
    return static_cast<CoclassType*>(type);
}

void CoclassType::AddConstant(
    /* [in] */ Constant* constant)
{
    if (constant != nullptr) {
        mConstants.push_back(constant);
    }
}

int CoclassType::GetConstantNumber()
{
    return mConstants.size();
}

String CoclassType::GetStrFramacBlock()
{
    return mStrFramacBlock;
}

} // namespace cdlc

#endif // __CDLC_COCLASSTYPE_H__
