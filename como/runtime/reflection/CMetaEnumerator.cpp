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

#include "reflection/CMetaEnumeration.h"
#include "reflection/CMetaEnumerator.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CMetaEnumerator, LightRefBase, IMetaEnumerator)

CMetaEnumerator::CMetaEnumerator(
    /* [in] */ CMetaEnumeration* menObj,
    /* [in] */ MetaEnumerator* me)
    : mMetadata(me)
    , mOwner(menObj)
    , mName(me->mName)
    , mValue(me->mValue)
{}

ECode CMetaEnumerator::GetEnumeration(
    /* [out] */ AutoPtr<IMetaEnumeration>& metaEnumn)
{
    metaEnumn = mOwner;
    return NOERROR;
}

ECode CMetaEnumerator::GetName(
    /* [out] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaEnumerator::GetValue(
    /* [out] */ Integer& value)
{
    value = mValue;
    return NOERROR;
}

} // namespace como
