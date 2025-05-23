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

#ifndef __COMO_OBJECT_H__
#define __COMO_OBJECT_H__

#include "comotypes.h"
#include "comoref.h"

namespace como {

class COM_PUBLIC Object
    : public RefBase
    , public IObject
    , public IWeakReferenceSource
{
public:
    COMO_INTERFACE_DECL();

    ECode AttachMetadata(
        /* [in] */ IMetaComponent* component,
        /* [in] */ const String& coclassName) override;

    ECode GetCoclassID(
        /* [out] */ CoclassID& cid) override;

    ECode GetCoclass(
        /* [out] */ AutoPtr<IMetaCoclass>& klass) override;

    ECode GetHashCode(
        /* [out] */ Long& hash) override;

    ECode GetCRC64(
        /* [out] */ Long& crc64) override;

    ECode Equals(
        /* [in] */ IInterface* obj,
        /* [out] */ Boolean& same) override;

    ECode SetObjectObserver(
        /* [in] */ IObjectObserver* observer) override;

    ECode ToString(
        /* [out] */ String& desc) override;

    ECode GetWeakReference(
        /* [out] */ AutoPtr<IWeakReference>& wr) override;

    void OnFirstRef(
        /* [in] */ const void* id) override;

    void OnLastStrongRef(
        /* [in] */ const void* id) override;

    void OnLastWeakRef(
        /* [in] */ const void* id) override;

    inline void SetObjSize(Integer objSize);

    static ECode GetCoclassID(
        /* [in] */ IInterface* obj,
        /* [out] */ CoclassID& cid);

    static AutoPtr<IMetaCoclass> GetCoclass(
        /* [in] */ IInterface* obj);

    static String GetFuncSafetySetting(
        /* [in] */ IInterface* obj);

    static String GetCoclassName(
        /* [in] */ IInterface* obj);

    static String GetCoclassName(
        /* [in] */ Object* obj);

    static Long GetHashCode(
        /* [in] */ IInterface* obj);

    static Long GetHashCode(
        /* [in] */ Object* obj);

    static Long GetCRC64(
        /* [in] */ IInterface* intf);

    static Long GetCRC64(
        /* [in] */ Object* obj);

    static Boolean Equals(
        /* [in] */ IInterface* obj1,
        /* [in] */ IInterface* obj2);

    static String ToString(
        /* [in] */ IInterface* obj);

    static AutoPtr<IWeakReference> GetWeakReference(
        /* [in] */ IInterface* obj);

    static Boolean InstanceOf(
        /* [in] */ IInterface* obj,
        /* [in] */ const CoclassID& cid);

    static Boolean InstanceOf(
        /* [in] */ Object* obj,
        /* [in] */ const CoclassID& cid);

    void TrackMe(
        /* [in] */ Boolean enable,
        /* [in] */ Boolean retain);

    void PrintRefs(const char* objInfo) const;

    inline static Object* From(IObject* obj);

    inline static void* ObjToRefBasePtr(IObject* obj);

    inline static void* IntfToRefBasePtr(IInterface* intf);

private:
    IMetaComponent* mComponent;
    String mCoclassName;
    AutoPtr<IObjectObserver> mObserver;
    Integer mObjSize;
};

/**
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
 * sizeof({vfptr}, HANDLE)      = 8
 *
 * The sizeof() operator returns the size of a particular type or object
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
#ifndef OBJECTSIZE_Offset
#define OBJECTSIZE_Offset (sizeof(RefBase) + sizeof(IObject) + \
                                                   sizeof(IWeakReferenceSource))
#define REFBASESIZE_Offset (sizeof(RefBase) + sizeof(IObject) + \
                                  sizeof(IWeakReferenceSource) + sizeof(HANDLE))
#endif

inline void Object::SetObjSize(Integer objSize)
{
    mObjSize = objSize;
}

Object* Object::From(IObject* obj)
{
    return static_cast<Object*>(obj);
}

/**
 * Calculate the RefBase object pointer from the IObject
 */
void* Object::ObjToRefBasePtr(IObject* obj)
{
    RefBase *refBase = (RefBase*)(Object*)obj;
    return (void*)refBase;
}

/**
 * Calculate the RefBase object pointer from the IInterface
 */
void* Object::IntfToRefBasePtr(IInterface* intf)
{
    RefBase *refBase = (RefBase*)(Object*)(IObject::Probe(intf));
    return (void*)refBase;
}

// generate a ComponentID from module name or
// a CoclassID from Coclass namespace::name or
// an InterfaceID from Interface namespace::name
COM_PUBLIC ComponentID ComponentIDfromName(String name, const char* uri);
COM_PUBLIC CoclassID CoclassIDfromName(String namespaceAndName, const ComponentID* componentID);
COM_PUBLIC InterfaceID InterfaceIDfromName(String namespaceAndName, const ComponentID* componentID);
COM_PUBLIC InterfaceID InterfaceIDfromNameWithMemArea(String namespaceAndName, Short iMemArea);
COM_PUBLIC InterfaceID InterfaceIDWithMemArea(const InterfaceID& iid, Short iMemArea);

} // namespace como

#endif // __COMO_OBJECT_H__
