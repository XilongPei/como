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

#include <time.h>
#include "registry.h"
#include "util/mutex.h"
#include "util/hashmapCache.h"

namespace como {

template<>
struct HashFunc<IObject*>
{
    inline Long operator()(
        /* [in] */ IObject* data)
    {
        Long hash;
        data->GetHashCode(hash);
        return hash;
    }
};

template<>
struct HashFunc<IInterfacePack*>
{
    inline Long operator()(
        /* [in] */ IInterfacePack* data)
    {
        Long hash;
        data->GetHashCode(hash);
        return hash;
    }
};

template<>
struct HashFunc<Long>
{
    inline Long operator()(
        /* [in] */ Long data)
    {
        return data;
    }
};

static HashMapCache<IObject*, IStub*> sLocalExportRegistry;
static Mutex sLocalExportRegistryLock;
static HashMapCache<IObject*, IStub*> sRemoteExportRegistry;
static Mutex sRemoteExportRegistryLock;

static HashMapCache<IInterfacePack*, IObject*> sLocalImportRegistry;
static Mutex sLocalImportRegistryLock;
static HashMapCache<IInterfacePack*, IObject*> sRemoteImportRegistry;
static Mutex sRemoteImportRegistryLock;

ECode RegisterExportObject(
    /* [in] */ RPCType type,
    /* [in] */ IObject* object,
    /* [in] */ IStub* stub)
{
    if (object == nullptr || stub == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    if (0 != registry.Put(object, stub))
        return E_ILLEGAL_ARGUMENT_EXCEPTION;

    return NOERROR;
}

ECode UnregisterExportObject(
    /* [in] */ RPCType type,
    /* [in] */ IObject* object)
{
    if (object == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = type == (RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(object)) {
        registry.Remove(object);
        return NOERROR;
    }
    return E_NOT_FOUND_EXCEPTION;
}

ECode UnregisterExportObject(
    /* [in] */ RPCType type,
    /* [in] */ Long hash)
{
    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = type == (RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    registry.Remove(hash);
    return NOERROR;
}

ECode FindExportObject(
    /* [in] */ RPCType type,
    /* [in] */ IObject* object,
    /* [out] */ AutoPtr<IStub>& stub)
{
    if (object == nullptr) {
        stub = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(object)) {
        stub = registry.Get(object);
        return NOERROR;
    }
    stub = nullptr;
    return E_NOT_FOUND_EXCEPTION;
}

ECode FindExportObject(
    /* [in] */ RPCType type,
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ AutoPtr<IStub>& stub)
{
    if (ipack == nullptr) {
        stub = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    Array<IStub*> stubs = registry.GetValues();
    for (Long i = 0;  i < stubs.GetLength();  i++) {
        IStub* stubObj = stubs[i];
        Boolean matched;
        stubObj->Match(ipack, matched);
        if (matched) {
            stub = stubObj;
            return NOERROR;
        }
    }
    stub = nullptr;
    return E_NOT_FOUND_EXCEPTION;
}

ECode RegisterImportObject(
    /* [in] */ RPCType type,
    /* [in] */ IInterfacePack* ipack,
    /* [in] */ IObject* object)
{
    if (ipack == nullptr || object == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    registry.Put(ipack, object);
    return NOERROR;
}

ECode UnregisterImportObject(
    /* [in] */ RPCType type,
    /* [in] */ IInterfacePack* ipack)
{
    if (ipack == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    // A temporary measure, if there are not many cached objects,
    // do not remove them first
    if (registry.GetDataNumber() < 100)
        return NOERROR;

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(ipack)) {
        registry.Remove(ipack);
        return NOERROR;
    }
    return E_NOT_FOUND_EXCEPTION;
}

ECode FindImportObject(
    /* [in] */ RPCType type,
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ AutoPtr<IObject>& object)
{
    if (ipack == nullptr) {
        object = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(ipack)) {
        object = registry.Get(ipack);
        return NOERROR;
    }
    object = nullptr;
    return E_NOT_FOUND_EXCEPTION;
}

static void ExportRegistryWalker(String& str, IObject* obj, IStub* istub)
{
    AutoPtr<IMetaCoclass> klass;
    obj->GetCoclass(klass);
    String name, ns;
    klass->GetName(name);
    klass->GetNamespace(ns);
    str += ("Namespace=" + ns + ";Name=" + name);
}

static void ImportRegistryWalker(String& str, IInterfacePack* intfPack, IObject* obj)
{
    AutoPtr<IMetaCoclass> klass;
    obj->GetCoclass(klass);
    String name, ns;
    klass->GetName(name);
    klass->GetNamespace(ns);
    str += ("Namespace=" + ns + ";Name=" + name);

    InterfaceID iid;
    String serverName;
    intfPack->GetInterfaceID(iid);
    intfPack->GetServerName(serverName);
    str += (";InterfaceID=" + DumpUUID(iid.mUuid) + ";ServerName=" + serverName);
}

ECode WalkExportObject(
    /* [in] */ RPCType type,
    /* [out] */ String& strBuffer)
{
    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    registry.GetValues(strBuffer, ExportRegistryWalker);
    return NOERROR;
}

ECode WalkImportObject(
    /* [in] */ RPCType type,
    /* [out] */ String& strBuffer)
{
    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    Mutex::AutoLock lock(registryLock);
    registry.GetValues(strBuffer, ImportRegistryWalker);
    return NOERROR;
}

} // namespace como
