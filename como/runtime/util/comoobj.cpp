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
#include "checksum.h"
#include "cityhash.h"
#include "int64.h"

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
    if (nullptr == mComponent) {
        klass = nullptr;    //AutoPtr<T>::operator= is overloaded.
                            //NOT: MISRA-C++: Pointer assignment to wider scope.
        return E_UNSUPPORTED_OPERATION_EXCEPTION;
    }
    return mComponent->GetCoclass(mCoclassName, klass);
}

/**
 * defined in comoobjapi.cpp
 */
extern Short g_iRuntimeID;

ECode Object::GetHashCode(
    /* [out] */ Long& hash)
{
    hash = reinterpret_cast<Long>(this);

    Int64 *int64 = reinterpret_cast<Int64 *>(&hash);
    int64->i16.i16_high_high = g_iRuntimeID;

    return NOERROR;
}

ECode Object::GetCRC64(
    /* [out] */ Long& crc64)
{
    /**
     * COMO objects (come from coclass, defined in CDL) are defined this way：
     * class C...       // such as: CArrayClassObject, coclass CArray in libcore
     *     : public ClassObject
     *     , public ...
     *
     * class COM_PUBLIC ClassObject
     *     : public Object
     *     , public IClassObject

     * Object is the first base class, therefore, we must make an
     * adjustment [- sizeof(Object)] to the current object pointer (this) in
     * order to get the original COMO object pointer.
     *
     * The inheritance relationship of Object is as follows:
     * class Object
     *     : public RefBase
     *     , public IObject
     *     , public IWeakReferenceSource
     *
     * sizeof(Object)               = 80
     * sizeof(RefBase)              = 24
     * sizeof(IObject)              = 8
     * sizeof(IWeakReferenceSource) = 8
     *
     * The sizeof() operator returns the size of a particular type or object,
     * in bytes. sizeof() does not include the sizeof() `this` pointer.
     * If a class has only virtual functions and no data members, its size is
     * the size of the virtual table pointer.
     *
     * (class Object):
     * +----+
     * 0    | +- (base class RefBase) [sizeof(RefBase) = 24]
     * 0    | {vfptr}
     * 8    | members of class RefBase
     *      +---
     * 24   | +- (base class IObject)[sizeof(IObject) = 8]
     * 24   | {vfptr}
     * 32   | members of class IObject
     *      +---
     * 32   | +- (base class IWeakReferenceSource) [sizeof(IWeakReferenceSource) = 8]
     * 32   | {vfptr}
     * 40   | members of class IWeakReferenceSource
     *      +---
     * 40   | +- (class Object)
     * 40   | {vfptr}
     * 48   | members of class Object
     */
    crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(this),
                                                  mObjSize - OBJECTSIZE_Offset);
    return NOERROR;
}

ECode Object::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& same)
{
    same = (IObject::Probe(obj) == (IObject*)this);
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
    FAIL_RETURN(GetCoclass(mc));

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
    if (nullptr == wr) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    return NOERROR;
}

void Object::OnFirstRef(
    /* [in] */ const void* id)
{
    if (UNLIKELY(mObserver != nullptr)) {
        mObserver->OnFirstRef(this, reinterpret_cast<HANDLE>(id));
    }
}

void Object::OnLastStrongRef(
    /* [in] */ const void* id)
{
    if (UNLIKELY(mObserver != nullptr)) {
        mObserver->OnLastStrongRef(this, reinterpret_cast<HANDLE>(id));
    }
}

void Object::OnLastWeakRef(
    /* [in] */ const void* id)
{
    if (UNLIKELY(mObserver != nullptr)) {
        mObserver->OnLastWeakRef(this, reinterpret_cast<HANDLE>(id));
    }
}

ECode Object::GetCoclassID(
    /* [in] */ IInterface* obj,
    /* [out] */ CoclassID& cid)
{
    IObject* objTmp = IObject::Probe(obj);
    if (nullptr == objTmp) {
        cid = CoclassID::Null;
        return NOERROR;
    }
    return objTmp->GetCoclassID(cid);
}

AutoPtr<IMetaCoclass> Object::GetCoclass(
    /* [in] */ IInterface* obj)
{
    IObject* objTmp = IObject::Probe(obj);
    if (objTmp != nullptr) {
        AutoPtr<IMetaCoclass> mc;
        ECode ec = objTmp->GetCoclass(mc);
        if (FAILED(ec)) {
            return nullptr;
        }

        return mc;
    }
    return nullptr;
}

String Object::GetFuncSafetySetting(
    /* [in] */ IInterface* obj)
{
    String funcSafetySetting;

    IObject* objTmp = IObject::Probe(obj);
    if (objTmp != nullptr) {
        AutoPtr<IMetaCoclass> mc;
        ECode ec = objTmp->GetCoclass(mc);
        if (FAILED(ec)) {
            return nullptr;
        }

        if (mc != nullptr) {
            ec = mc->GetFuncSafetySetting(funcSafetySetting);
            if (UNLIKELY(FAILED(ec))) {               // This shouldn't happen
                return nullptr;
            }
        }

        return funcSafetySetting;
    }
    return nullptr;
}

String Object::GetCoclassName(
    /* [in] */ IInterface* obj)
{
    Object* objTmp = static_cast<Object*>(IObject::Probe(obj));
    if (nullptr == objTmp) {
        return nullptr;
    }
    return GetCoclassName(objTmp);
}

String Object::GetCoclassName(
    /* [in] */ Object* obj)
{
    String name;
    AutoPtr<IMetaCoclass> mc;
    ECode ec = obj->GetCoclass(mc);
    if (FAILED(ec)) {
        return String("");
    }

    if (mc != nullptr) {
        mc->GetName(name);
    }
    return name;
}

Long Object::GetHashCode(
    /* [in] */ IInterface* obj)
{
    Object* objTmp = (Object*)IObject::Probe(obj);
    if (nullptr == objTmp) {
        return reinterpret_cast<uintptr_t>(IInterface::Probe(obj));
    }
    return GetHashCode(objTmp);
}

Long Object::GetHashCode(
    /* [in] */ Object* obj)
{
    Long hash;
    obj->GetHashCode(hash);
    return hash;
}

Long Object::GetCRC64(
    /* [in] */ IInterface* intf)
{
    // Sometimes, for example, in function safety calculation, because the object
    // in the object inherits ComoFunctionSafetyObject, Object* is not the header
    // pointer of the whole object
    Object* obj = static_cast<Object*>(IObject::Probe(intf));
    if (nullptr == obj) {
        return 0L;
    }

    // reference to: Object::GetCRC64(Long& crc64)
    return crc_64_ecma(reinterpret_cast<const unsigned char *>(obj),
                                             obj->mObjSize - OBJECTSIZE_Offset);
}

Long Object::GetCRC64(
    /* [in] */ Object* obj)
{
    Long crc64;
    obj->GetCRC64(crc64);
    return crc64;
}

Boolean Object::Equals(
    /* [in] */ IInterface* obj1,
    /* [in] */ IInterface* obj2)
{
    if (IInterface::Probe(obj1) == IInterface::Probe(obj2)) {
        return true;
    }

    IObject* o1 = IObject::Probe(obj1);
    if (nullptr == o1) {
        return false;
    }

    Boolean result;
    o1->Equals(obj2, result);
    return result;
}

String Object::ToString(
    /* [in] */ IInterface* obj)
{
    if (nullptr == obj) {
        return "null";
    }
    else {
        IObject* objTmp = IObject::Probe(obj);
        if (objTmp != nullptr) {
            String info;
            objTmp->ToString(info);
            return info;
        }
        return "not a coclass object.";
    }
}

AutoPtr<IWeakReference> Object::GetWeakReference(
    /* [in] */ IInterface* obj)
{
    IWeakReferenceSource* wrSource = IWeakReferenceSource::Probe(obj);
    if (nullptr == wrSource) {
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
    Object *objTmp = static_cast<Object*>(IObject::Probe(obj));
    if (nullptr == objTmp) {
        return false;
    }
    return InstanceOf(objTmp, cid);
}

Boolean Object::InstanceOf(
    /* [in] */ Object* obj,
    /* [in] */ const CoclassID& cid)
{
    CoclassID ocid;
    obj->GetCoclassID(ocid);
    return (ocid == cid);
}

void Object::TrackMe(
    /* [in] */ Boolean enable,
    /* [in] */ Boolean retain)
{
    return RefBase::TrackMe(enable, retain);
}

void Object::PrintRefs(const char* objInfo) const
{
    return RefBase::PrintRefs(objInfo);
}

ComponentID ComponentIDfromName(String name, const char* uri)
{
    ComponentID mid;
#ifdef __SIZEOF_INT128__
    *(uint128 *)&mid.mUuid.mData1 = CityHash128(name, name.GetByteLength());
#else
    uint128 i128 = CityHash128(name, name.GetByteLength());
    memcpy((Byte*)&mid.mUuid.mData1, (Byte*)&i128, sizeof(uint128));
#endif
    mid.mUri = uri;
    return mid;
}

CoclassID CoclassIDfromName(String namespaceAndName, const ComponentID* componentID)
{
    CoclassID cid;
#ifdef __SIZEOF_INT128__
    *(uint128 *)&cid.mUuid.mData1 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
#else
    uint128 i128 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
    memcpy((Byte*)&cid.mUuid.mData1, (Byte*)&i128, sizeof(uint128));
#endif
    cid.mCid = componentID;
    return cid;
}

InterfaceID InterfaceIDfromName(String namespaceAndName, const ComponentID* componentID)
{
    InterfaceID iid;
#ifdef __SIZEOF_INT128__
    *(uint128 *)&iid.mUuid.mData1 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
#else
    uint128 i128 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
    memcpy((Byte*)&iid.mUuid.mData1, (Byte*)&i128, sizeof(uint128));
#endif
    iid.mCid = componentID;
    return iid;
}

InterfaceID InterfaceIDfromNameWithMemArea(String namespaceAndName, Short iMemArea)
{
    InterfaceID iid;
#ifdef __SIZEOF_INT128__
    *(uint128 *)&iid.mUuid.mData1 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
#else
    uint128 i128 = CityHash128(namespaceAndName, namespaceAndName.GetByteLength());
    memcpy((Byte*)&iid.mUuid.mData1, (Byte*)&i128, sizeof(uint128));
#endif

    if ((iMemArea < 0) || (iMemArea > 4096)) {
        iid.mCid = nullptr;
    }
    else {
        iid.mCid = (const ComponentID*)(static_cast<HANDLE>(iMemArea + 1));
    }

    return iid;
}

InterfaceID InterfaceIDWithMemArea(const InterfaceID& iid, Short iMemArea)
{
    InterfaceID value;

    value = iid;
    if ((iMemArea < 0) || (iMemArea > 4096)) {
        value.mCid = nullptr;
    }
    else {
        value.mCid = (const ComponentID*)(static_cast<HANDLE>(iMemArea + 1));
    }

    return value;
}

} // namespace como
