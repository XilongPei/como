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

private:
    IMetaComponent* mComponent;
    String mCoclassName;
    AutoPtr<IObjectObserver> mObserver;
    Integer mObjSize;
};

inline void Object::SetObjSize(Integer objSize)
{
    mObjSize = objSize;
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
