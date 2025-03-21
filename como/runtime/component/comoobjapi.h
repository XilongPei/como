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

#ifndef __COMO_COMOOBJECTAPI_H__
#define __COMO_COMOOBJECTAPI_H__

#include "comointfs.h"

namespace como {

EXTERN_C COM_PUBLIC ECode CoCreateObjectInstance(
    /* [in] */ const CoclassID& cid,
    /* [in] */ const InterfaceID& iid,
    /* [in] */ IClassLoader* loader,
    /* [out] */ IInterface** object);

EXTERN_C COM_PUBLIC ECode CoAcquireClassFactory(
    /* [in] */ const CoclassID& cid,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IClassObject>& object);

EXTERN_C COM_PUBLIC ECode CoGetBootClassLoader(
    /* [out] */ AutoPtr<IClassLoader>& loader);

EXTERN_C COM_PUBLIC ECode CoSetSystemClassLoader(
    /* [out] */ IClassLoader* loader);

EXTERN_C COM_PUBLIC ECode CoClassForName(
    /* [in] */ String& namespaceAndName,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IMetaCoclass>& mc);

EXTERN_C COM_PUBLIC ECode CoSetRuntimeID(
    /* [in] */ Short iRuntimeID);

} // namespace como

#endif // __COMO_COMOOBJECTAPI_H__
