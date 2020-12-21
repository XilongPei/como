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

#include "comoobj.h"

namespace como {

Integer Object::AddRef(
    /* [in] */ HANDLE id)
{
    return RefBase::AddRef(id);
}

Integer Object::Release(
    /* [in] */ HANDLE id)
{
    return RefBase::Release(id);
}

IInterface* Object::Probe(
    /* [in] */ const InterfaceID& iid)
{
    if (iid == IID_IInterface) {
        return (IInterface*)(IObject*)this;
    }
    else if (iid == IID_IObject) {
        return (IObject*)this;
    }
    else if (iid == IID_IWeakReferenceSource) {
        return (IWeakReferenceSource*)this;
    }
    return nullptr;
}

ECode Object::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID& iid)
{
    if (object == (IInterface*)(IObject*)this) {
        iid = IID_IObject;
    }
    else if (object == (IWeakReferenceSource*)this) {
        iid = IID_IWeakReferenceSource;
    }
    else {
        iid = InterfaceID::Null;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return NOERROR;
}

ECode Object::AttachMetadata(
    /* [in] */ IMetaComponent* component,
    /* [in] */ const String& coclassName)
{
    mComponent = component;
    mCoclassName = coclassName;
    return NOERROR;
}

ECode Object::GetCoclassID(
    /* [out] */ CoclassID& cid)
{
    cid = CoclassID::Null;
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode Object::GetCoclass(
    /* [out] */ AutoPtr<IMetaCoclass>& klass)
{
    if (mComponent == nullptr) {
        klass = nullptr;
        return E_UNSUPPORTED_OPERATION_EXCEPTION;
    }
    return mComponent->GetCoclass(mCoclassName, klass);
}

ECode Object::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = reinterpret_cast<HANDLE>(this);
    return NOERROR;
}

ECode Object::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& same)
{
    same = IObject::Probe(obj) == (IObject*)this;
    return NOERROR;
}

ECode Object::SetObjectObserver(
    /* [in] */ IObjectObserver* observer)
{
    mObserver = observer;
    return NOERROR;
}

ECode Object::ToString(
    /* [out] */ String& desc)
{
    AutoPtr<IMetaCoclass> mc;
    GetCoclass(mc);
    String ns, name;
    if (mc != nullptr) {
        mc->GetNamespace(ns);
        mc->GetName(name);
    }
    desc = String::Format("Object[0x%x], Class[%s%s]",
            this, ns.string(), name.string());
    return NOERROR;
}

ECode Object::GetWeakReference(
    /* [out] */ AutoPtr<IWeakReference>& wr)
{
    wr = new WeakReferenceImpl((IObject*)this, CreateWeak(this));
    return NOERROR;
}

void Object::OnLastStrongRef(
    /* [in] */ const void* id)
{
    if (UNLIKELY(mObserver != nullptr)) {
        mObserver->OnLastStrongRef(this);
    }
}

void Object::OnLastWeakRef(
    /* [in] */ const void* id)
{
    if (UNLIKELY(mObserver != nullptr)) {
        mObserver->OnLastWeakRef(this);
    }
}

ECode Object::GetCoclassID(
    /* [in] */ IInterface* obj,
    /* [out] */ CoclassID& cid)
{
    IObject* o = IObject::Probe(obj);
    if (o == nullptr) {
        cid = CoclassID::Null;
        return NOERROR;
    }
    return o->GetCoclassID(cid);
}

AutoPtr<IMetaCoclass> Object::GetCoclass(
    /* [in] */ IInterface* obj)
{
    IObject* o = IObject::Probe(obj);
    if (o != nullptr) {
        AutoPtr<IMetaCoclass> mc;
        o->GetCoclass(mc);
        return mc;
    }
    return nullptr;
}

String Object::GetCoclassName(
    /* [in] */ IInterface* obj)
{
    Object* o = static_cast<Object*>(IObject::Probe(obj));
    if (o == nullptr) {
        return nullptr;
    }
    return GetCoclassName(o);
}

String Object::GetCoclassName(
    /* [in] */ Object* obj)
{
    String name;
    AutoPtr<IMetaCoclass> mc;
    obj->GetCoclass(mc);
    if (mc != nullptr) {
        mc->GetName(name);
    }
    return name;
}

Integer Object::GetHashCode(
    /* [in] */ IInterface* obj)
{
    Object* o = (Object*)IObject::Probe(obj);
    if (o == nullptr) {
        return reinterpret_cast<uintptr_t>(IInterface::Probe(obj));
    }
    return GetHashCode(o);
}

Integer Object::GetHashCode(
    /* [in] */ Object* obj)
{
    Integer hash;
    obj->GetHashCode(hash);
    return hash;
}

Boolean Object::Equals(
    /* [in] */ IInterface* obj1,
    /* [in] */ IInterface* obj2)
{
    if (IInterface::Probe(obj1) == IInterface::Probe(obj2)) {
        return true;
    }

    IObject* o1 = IObject::Probe(obj1);
    if (o1 == nullptr) {
        return false;
    }
    Boolean result;
    o1->Equals(obj2, result);
    return result;
}

String Object::ToString(
    /* [in] */ IInterface* obj)
{
    if (obj == nullptr) {
        return "null";
    }
    else {
        IObject* o = IObject::Probe(obj);
        if (o != nullptr) {
            String info;
            o->ToString(info);
            return info;
        }
        return "not a coclass object.";
    }
}

AutoPtr<IWeakReference> Object::GetWeakReference(
    /* [in] */ IInterface* obj)
{
    IWeakReferenceSource* wrSource = IWeakReferenceSource::Probe(obj);
    if (wrSource == nullptr) {
        return nullptr;
    }
    AutoPtr<IWeakReference> wr;
    wrSource->GetWeakReference(wr);
    return wr;
}

Boolean Object::InstanceOf(
    /* [in] */ IInterface* obj,
    /* [in] */ const CoclassID& cid)
{
    Object* o = static_cast<Object*>(IObject::Probe(obj));
    if (o == nullptr) {
        return false;
    }
    return InstanceOf(o, cid);
}

Boolean Object::InstanceOf(
    /* [in] */ Object* obj,
    /* [in] */ const CoclassID& cid)
{
    CoclassID ocid;
    obj->GetCoclassID(ocid);
    return ocid == cid;
}

}
