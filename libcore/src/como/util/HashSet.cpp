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

#include "como/core/Math.h"
#include "como/util/CHashMap.h"
#include "como/util/CLinkedHashMap.h"
#include "como/util/HashSet.h"

using como::core::IID_ICloneable;
using como::core::Math;
using como::io::IID_ISerializable;

namespace como {
namespace util {

COMO_INTERFACE_IMPL_3(HashSet, AbstractSet, IHashSet, ICloneable, ISerializable);

AutoPtr<IInterface> HashSet::GetPRESENT()
{
    static AutoPtr<IObject> PRESENT = new Object();
    return PRESENT.Get();
}

ECode HashSet::Constructor()
{
    CHashMap::New(IID_IHashMap, (IInterface**)&mMap);
    return NOERROR;
}

ECode HashSet::Constructor(
    /* [in] */ ICollection* c)
{
    Integer size;
    c->GetSize(size);
    CHashMap::New(Math::Max((Integer)(size / .75f) + 1, 16),
            IID_IHashMap, (IInterface**)&mMap);
    AddAll(c);
    return NOERROR;
}

ECode HashSet::Constructor(
    /* [in] */ Integer initialCapacity,
    /* [in] */ Float loadFactor)
{
    CHashMap::New(initialCapacity, loadFactor,
            IID_IHashMap, (IInterface**)&mMap);
    return NOERROR;
}

ECode HashSet::Constructor(
    /* [in] */ Integer initialCapacity)
{
    CHashMap::New(initialCapacity, IID_IHashMap, (IInterface**)&mMap);
    return NOERROR;
}

ECode HashSet::Constructor(
    /* [in] */ Integer initialCapacity,
    /* [in] */ Float loadFactor,
    /* [in] */ Boolean dummy)
{
    CLinkedHashMap::New(initialCapacity, loadFactor,
            IID_IHashMap, (IInterface**)&mMap);
    return NOERROR;
}

ECode HashSet::GetIterator(
    /* [out] */ AutoPtr<IIterator>& it)
{
    AutoPtr<ISet> keys;
    mMap->GetKeySet(keys);
    return keys->GetIterator(it);
}

ECode HashSet::GetSize(
    /* [out] */ Integer& size)
{
    return mMap->GetSize(size);
}

ECode HashSet::IsEmpty(
    /* [out] */ Boolean& result)
{
    return mMap->IsEmpty(result);
}

ECode HashSet::Contains(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& result)
{
    return mMap->ContainsKey(obj, result);
}

ECode HashSet::Add(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* modified)
{
    AutoPtr<IInterface> prevVal;
    mMap->Put(obj, GetPRESENT(), &prevVal);
    if (modified != nullptr) {
        *modified = prevVal == nullptr;
    }
    return NOERROR;
}

ECode HashSet::Remove(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* contained)
{
    AutoPtr<IInterface> prevVal;
    mMap->Remove(obj, &prevVal);
    if (contained != nullptr) {
        *contained = IInterface::Equals(prevVal, GetPRESENT());
    }
    return NOERROR;
}

ECode HashSet::Clear()
{
    return mMap->Clear();
}

ECode HashSet::CloneImpl(
    /* [in] */ IHashSet* newObj)
{
    HashSet* set = (HashSet*)newObj;
    AutoPtr<IHashMap> map;
    FAIL_RETURN(ICloneable::Probe(set->mMap)->Clone(IID_IHashMap, (IInterface**)&map));
    mMap = map;
    return NOERROR;
}

ECode HashSet::AddAll(
    /* [in] */ ICollection* c,
    /* [out] */ Boolean* changed)
{
    return AbstractSet::AddAll(c, changed);
}

ECode HashSet::ContainsAll(
    /* [in] */ ICollection* c,
    /* [out] */ Boolean& result)
{
    return AbstractSet::ContainsAll(c, result);
}

ECode HashSet::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& result)
{
    return AbstractSet::Equals(obj, result);
}

ECode HashSet::GetHashCode(
    /* [out] */ Integer& hash)
{
    return AbstractSet::GetHashCode(hash);
}

ECode HashSet::RemoveAll(
    /* [in] */ ICollection* c,
    /* [out] */ Boolean* changed)
{
    return AbstractSet::RemoveAll(c, changed);
}

ECode HashSet::RetainAll(
    /* [in] */ ICollection* c,
    /* [out] */ Boolean* changed)
{
    return AbstractSet::RetainAll(c, changed);
}

ECode HashSet::ToArray(
    /* [out, callee] */ Array<IInterface*>* objs)
{
    return AbstractSet::ToArray(objs);
}

ECode HashSet::ToArray(
    /* [in] */ const InterfaceID& iid,
    /* [out, callee] */ Array<IInterface*>* objs)
{
    return AbstractSet::ToArray(iid, objs);
}

}
}
