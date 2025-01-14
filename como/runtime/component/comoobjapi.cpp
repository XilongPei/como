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

#include "comocomp.h"
#include "comoobjapi.h"
#include "CBootClassLoader.h"
#include "reflection/CMetaComponent.h"
#include "util/comosp.h"

namespace como {

/**
 * COMO runtime ID, which uniquely identifies the COMO runtime. This ID is used
 * to distinguish the COMO running instances when there are multiple COMO running
 * instances, especially those running on the same operating system.
 */
Short g_iRuntimeID = 0;

ECode CoCreateObjectInstance(
    /* [in] */ const CoclassID& cid,
    /* [in] */ const InterfaceID& iid,
    /* [in] */ IClassLoader* loader,
    /* [out] */ IInterface** object)
{
    VALIDATE_NOT_NULL(object);

    if (nullptr == loader) {
        loader = CBootClassLoader::GetSystemClassLoader();
    }

    AutoPtr<IClassObject> factory;
    ECode ec = CoAcquireClassFactory(cid, loader, factory);
    if (FAILED(ec)) {
        *object = nullptr;
        return ec;
    }

    return factory->CreateObject(iid, object);
}

ECode CoAcquireClassFactory(
    /* [in] */ const CoclassID& cid,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IClassObject>& object)
{
    if (nullptr == loader) {
        loader = CBootClassLoader::GetSystemClassLoader();
    }

    AutoPtr<IMetaComponent> component;
    ECode ec = loader->LoadComponent(*cid.mCid, component);
    if (FAILED(ec)) {
        object = nullptr;
        return ec;
    }

    CMetaComponent* mcObj = static_cast<CMetaComponent*>(component.Get());
    ec = mcObj->GetClassObject(cid, object);
    if (SUCCEEDED(ec)) {
        object->AttachMetadata(component);
    }
    return ec;
}

ECode CoGetBootClassLoader(
    /* [out] */ AutoPtr<IClassLoader>& loader)
{
    loader = CBootClassLoader::GetInstance();
    return NOERROR;
}

ECode CoSetSystemClassLoader(
    /* [out] */ IClassLoader* loader)
{
    if (loader != nullptr) {
        CBootClassLoader::SetSystemClassLoader(loader);
    }
    return NOERROR;
}

/**
 * Find coclass in the components already cached by LoadComponent().
 */
ECode CoClassForName(
    /* [in] */ String& namespaceAndName,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IMetaCoclass>& mc)
{
    if (nullptr == loader) {
        loader = CBootClassLoader::GetSystemClassLoader();
    }

    CoclassID cid;

    // LoadCoclass will not attempt to call LoadComponent, so let
    // cid.mCid (componentID) be nullptr.
    cid = CoclassIDfromName(namespaceAndName, nullptr);

    return loader->LoadCoclass(cid, mc);
}

ECode CoSetRuntimeID(Short iRuntimeID)
{
    g_iRuntimeID = iRuntimeID;
    return NOERROR;
}

} // namespace como
