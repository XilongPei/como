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

#ifndef __COMO_CORE_CLASSLOADER_H__
#define __COMO_CORE_CLASSLOADER_H__

#include "como/core/SyncObject.h"
#include "como.io.IInputStream.h"
#include "como.util.IHashMap.h"
#include <comosp.h>

using como::io::IInputStream;
using como::util::IHashMap;

namespace como {
namespace core {

class COM_PUBLIC ClassLoader
    : public SyncObject
    , public IClassLoader
{
private:
    class SystemClassLoader
    {
    public:
        static AutoPtr<IClassLoader> GetInstance();
    };

public:
    COMO_INTERFACE_DECL();

    ECode LoadCoclass(
        /* [in] */ const String& fullName,
        /* [out] */ AutoPtr<IMetaCoclass>& klass) override;

    ECode LoadCoclass(
        /* [in] */ const CoclassID& cid,
        /* [out] */ AutoPtr<IMetaCoclass>& klass) override
    {
        return NOERROR;
    }

    ECode GetParent(
        /* [out] */ AutoPtr<IClassLoader>& parent) override;

    static AutoPtr<IClassLoader> GetSystemClassLoader();

    ECode LoadInterface(
        /* [in] */ const String& fullName,
        /* [out] */ AutoPtr<IMetaInterface>& intf) override;

    ECode LoadComponent(
        /* [in] */ const String& path,
        /* [out] */ AutoPtr<IMetaComponent>& component) override;

    ECode LoadComponent(
        /* [in] */ const ComponentID& compId,
        /* [out] */ AutoPtr<IMetaComponent>& component) override;

    ECode UnloadComponent(
        /* [in] */ const ComponentID& compId) override;

    ECode LoadMetadata(
        /* [in] */ const Array<Byte>& metadata,
        /* [out] */ AutoPtr<IMetaComponent>& component) override
    {
        return NOERROR;
    }

    static AutoPtr<IInputStream> GetSystemResourceAsStream(
        /* [in] */ const String& name);

protected:
    ECode Constructor(
        /* [in] */ IClassLoader* parent);

    virtual ECode FindCoclass(
        /* [in] */ const String& fullName,
        /* [out] */ AutoPtr<IMetaCoclass>& klass);

    virtual AutoPtr<IMetaCoclass> FindLoadedCoclass(
        /* [in] */ const String& fullName);

    virtual ECode FindInterface(
        /* [in] */ const String& fullName,
        /* [out] */ AutoPtr<IMetaInterface>& intf);

    virtual AutoPtr<IMetaInterface> FindLoadedInterface(
        /* [in] */ const String& fullName);

private:
    static AutoPtr<IClassLoader> CreateSystemClassLoader();

private:
    AutoPtr<IClassLoader> mParent;

    AutoPtr<IHashMap> mLoadedCoclasses;

    AutoPtr<IHashMap> mLoadedInterfaces;
};

inline AutoPtr<IClassLoader> ClassLoader::GetSystemClassLoader()
{
    return SystemClassLoader::GetInstance();
}

}
}

#endif // __COMO_CORE_CLASSLOADER_H__
