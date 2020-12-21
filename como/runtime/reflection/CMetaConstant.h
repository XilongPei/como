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

#ifndef __COMO_CMETACONSTANT_H__
#define __COMO_CMETACONSTANT_H__

#include "comotypes.h"
#include "metadata/Component.h"
#include "util/comoref.h"

namespace como {

class CMetaConstant
    : public LightRefBase
    , public IMetaConstant
{
public:
    CMetaConstant(
        /* [in] */ MetaComponent* mc,
        /* [in] */ MetaConstant* mk);

    COMO_INTERFACE_DECL();

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetNamespace(
        /* [in] */ String& ns) override;

    ECode GetType(
        /* [out] */ AutoPtr<IMetaType>& type) override;

    ECode GetValue(
        /* [out] */ AutoPtr<IMetaValue>& value) override;

private:
    AutoPtr<IMetaValue> BuildValue(
        /* [in] */ IMetaType* type);

public:
    MetaConstant* mMetadata;
    String mName;
    String mNamespace;
    AutoPtr<IMetaType> mType;
    AutoPtr<IMetaValue> mValue;

private:
    static const char* TAG;
};

} // namespace como

#endif // __COMO_CMETACONSTANT_H__
