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

#ifndef __COMO_CMETAPARAMETER_H__
#define __COMO_CMETAPARAMETER_H__

#include "comotypes.h"
#include "metadata/Component.h"
#include "util/comoref.h"

namespace como {

class CMetaParameter
    : public LightRefBase
    , public IMetaParameter
{
public:
    CMetaParameter();

    CMetaParameter(
        /* [in] */ MetaComponent* mc,
        /* [in] */ IMetaMethod* mmObj,
        /* [in] */ MetaParameter* mp,
        /* [in] */ Integer index);

    COMO_INTERFACE_DECL();

    ECode GetMethod(
        /* [out] */ AutoPtr<IMetaMethod>& method) override;

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetIndex(
        /* [out] */ Integer& index) override;

    ECode GetIOAttribute(
        /* [out] */ IOAttribute& attr) override;

    ECode GetType(
        /* [out] */ AutoPtr<IMetaType>& type) override;

private:
    IOAttribute BuildIOAttribute(
        /* [in] */ unsigned char properties);

private:
    static constexpr int IN = 0x1;
    static constexpr int OUT = 0x2;
    static constexpr int CALLEE = 0x4;

public:
    MetaParameter* mMetadata;
    IMetaMethod* mOwner;
    String mName;
    Integer mIndex;
    IOAttribute mIOAttr;
    AutoPtr<IMetaType> mType;
};

} // namespace como

#endif // __COMO_CMETAPARAMETER_H__
