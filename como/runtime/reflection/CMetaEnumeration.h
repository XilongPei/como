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

#ifndef __COMO_CMETAENUMERATION_H__
#define __COMO_CMETAENUMERATION_H__

#include "metadata/Component.h"
#include "util/comoref.h"
#include "util/mutex.h"

namespace como {

class CMetaComponent;

class CMetaEnumeration
    : public LightRefBase
    , public IMetaEnumeration
{
public:
    CMetaEnumeration(
        /* [in] */ CMetaComponent* mcObj,
        /* [in] */ MetaComponent* mc,
        /* [in] */ MetaEnumeration* me);

    COMO_INTERFACE_DECL();

    ECode GetComponent(
        /* [out] */ AutoPtr<IMetaComponent>& comp) override;

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetNamespace(
        /* [out] */ String& ns) override;

    ECode GetEnumeratorNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllEnumerators(
        /* [out] */ Array<IMetaEnumerator*>& enumrs) override;

    ECode GetEnumerator(
        /* [in] */ const String& name,
        /* [out] */ AutoPtr<IMetaEnumerator>& enumr) override;

    void BuildAllEnumerators();

public:
    MetaEnumeration* mMetadata;
    CMetaComponent* mOwner;
    String mName;
    String mNamespace;
    Array<IMetaEnumerator*> mEnumerators;
    Mutex mEnumeratorsLock;
};

} // namespace como

#endif // __COMO_CMETAENUMERATION_H__
