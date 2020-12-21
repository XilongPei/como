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

#ifndef __COMO_CMETAENUMERATOR_H__
#define __COMO_CMETAENUMERATOR_H__

#include "metadata/Component.h"
#include "util/comoref.h"

namespace como {

class CMetaEnumeration;

class CMetaEnumerator
    : public LightRefBase
    , public IMetaEnumerator
{
public:
    CMetaEnumerator(
        /* [in] */ CMetaEnumeration* menObj,
        /* [in] */ MetaEnumerator* me);

    COMO_INTERFACE_DECL();

    ECode GetEnumeration(
        /* [out] */ AutoPtr<IMetaEnumeration>& metaEnumn) override;

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetValue(
        /* [out] */ Integer& value) override;

public:
    MetaEnumerator* mMetadata;
    CMetaEnumeration* mOwner;
    String mName;
    Integer mValue;
};

} // namespace como

#endif // __COMO_CMETAENUMERATOR_H__
