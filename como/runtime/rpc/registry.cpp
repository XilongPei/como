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
#include "reflection/reflHelpers.h"

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
    /* [in] */ IStub* stub,
    /* [in] */ Long value)
{
    if (object == nullptr || stub == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String strBuffer;
        Long hash;
        reflHelpers::intfGetObjectInfo(object, strBuffer);
        object->GetHashCode(hash);
        Logger::D("RegisterExportObject", "Object of %s, HashCode is 0x%llX",
                                                      strBuffer.string(), hash);
    }

    Mutex::AutoLock lock(registryLock);
    if (0 != registry.Put(object, stub, value)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

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
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String strBuffer;
        Long hash;
        reflHelpers::intfGetObjectInfo(object, strBuffer);
        object->GetHashCode(hash);
        Logger::D("UnregisterExportObject", "Object of %s, HashCode is 0x%llX",
                                                      strBuffer.string(), hash);
    }

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(object)) {
        registry.Remove(object);
        return NOERROR;
    }
    return E_NOT_FOUND_EXCEPTION;
}

ECode UnregisterExportObjectByHash(
    /* [in] */ RPCType type,
    /* [in] */ Long hash)
{
    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Logger::D("UnregisterExportObject", "HashCode is 0x%llX", hash);
    }

    Mutex::AutoLock lock(registryLock);
    if (! registry.RemoveByHash(hash)) {
        return E_NOT_FOUND_EXCEPTION;
    }
    return NOERROR;
}

ECode UnregisterExportObjectByChannel(
    /* [in] */ RPCType type,
    /* [in] */ Long channel)
{
    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Logger::D("UnregisterExportObjectByChannel", "channel: 0x%llX", channel);
    }

    Mutex::AutoLock lock(registryLock);
    if (! registry.RemoveByValue(channel)) {
        return E_NOT_FOUND_EXCEPTION;
    }
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

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String strBuffer;
        Long hash;
        reflHelpers::intfGetObjectInfo(object, strBuffer);
        object->GetHashCode(hash);
        Logger::D("FindExportObject", "Object of %s, HashCode is 0x%llX",
                                                      strBuffer.string(), hash);
    }

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(object)) {
        stub = registry.Get(object);
        return NOERROR;
    }
    stub = nullptr;
    return E_NOT_FOUND_EXCEPTION;
}

static Boolean Cmp_PVoid(void *p1, void *p2)
{
    Boolean matched;
    ((IStub*)p1)->Match((IInterfacePack*)p2, matched);
    return matched;
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

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Long objectId;
        ipack->GetServerObjectId(objectId);
        Logger::D("FindExportObject", "Object ServerObjectId: 0x%llX", objectId);
    }

    Mutex::AutoLock lock(registryLock);

    stub = registry.GetValue(Cmp_PVoid, (void*)ipack);
    if (nullptr != stub) {
        return NOERROR;
    }

    return E_NOT_FOUND_EXCEPTION;

// old algorithm
#if 0
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
#endif
}

ECode RegisterImportObject(
    /* [in] */ RPCType type,
    /* [in] */ IInterfacePack* ipack,
    /* [in] */ IObject* object,
    /* [in] */ Long value)
{
    if (ipack == nullptr || object == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Long objectId;
        ipack->GetServerObjectId(objectId);
        Logger::D("RegisterImportObject", "Object ServerObjectId: 0x%llX", objectId);
    }

    Mutex::AutoLock lock(registryLock);
    if (0 == registry.Put(ipack, object, value)) {
        return NOERROR;
    }

    return E_OUT_OF_MEMORY_ERROR;
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

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Long objectId;
        ipack->GetServerObjectId(objectId);
        Logger::D("UnregisterImportObject", "Object ServerObjectId: 0x%llX", objectId);
    }

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

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Long objectId;
        ipack->GetServerObjectId(objectId);
        Logger::D("FindImportObject", "Object ServerObjectId: 0x%llX", objectId);
    }

    Mutex::AutoLock lock(registryLock);
    if (registry.ContainsKey(ipack)) {
        object = registry.Get(ipack);
        return NOERROR;
    }
    object = nullptr;
    return E_NOT_FOUND_EXCEPTION;
}

ECode UnregisterImportObjectByChannel(
    /* [in] */ RPCType type,
    /* [in] */ Long channel)
{
    HashMapCache<IInterfacePack*, IObject*>& registry = (type == RPCType::Local) ?
                                   sLocalImportRegistry : sRemoteImportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalImportRegistryLock : sRemoteImportRegistryLock;

    if (Logger::GetLevel() <= Logger::DEBUG) {
        Logger::D("UnregisterImportObjectByChannel", "channel: 0x%llX", channel);
    }

    Mutex::AutoLock lock(registryLock);
    if (! registry.RemoveByValue(channel)) {
        return E_NOT_FOUND_EXCEPTION;
    }
    return NOERROR;
}

static void timespecToString(struct timespec& ts, const char *buffer, int bufSize)
{
    #if 0
    // For converting to data structure timeval, so mcroseconds are used here

        struct timeval {
            __time_t tv_sec;        /* Seconds. */
            __suseconds_t tv_usec;  /* Microseconds. */
        };
    #endif
    Long microseconds = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
                                  // 654321
    snprintf((char*)buffer, bufSize, "%lld", microseconds);
}

static void ExportRegistryWalker(String& str, IObject*& obj, IStub*& istub,
                                                struct timespec& lastAccessTime)
{
    char buf[32];
    timespecToString(lastAccessTime, buf, sizeof(buf));

    AutoPtr<IMetaCoclass> klass;
    obj->GetCoclass(klass);
    String name, ns;
    klass->GetName(name);
    klass->GetNamespace(ns);
    str += ("{\"Namespace\":\"" + ns + "\",\"CoclassName\":\"" + name + "\"" +
                                           ",\"LastAccessTime\":" + buf + "},");
}

static void ImportRegistryWalker(String& str, IInterfacePack*& intfPack,
                                  IObject*& obj, struct timespec& lastAccessTime)
{
    char buf[32];
    timespecToString(lastAccessTime, buf, sizeof(buf));

    AutoPtr<IMetaCoclass> klass;
    obj->GetCoclass(klass);
    String name, ns;
    klass->GetName(name);
    klass->GetNamespace(ns);
    str += ("{\"Namespace\":\"" + ns + "\",\"CoclassName\":\"" + name + "\",");

    InterfaceID iid;
    String serverName;
    intfPack->GetInterfaceID(iid);
    intfPack->GetServerName(serverName);
    str += ("\"InterfaceID\":\"" + DumpUUID(iid.mUuid) +
                                          "\",\"ServerName\":\"" + serverName +
                                          ",\"LastAccessTime\":" + buf + "},");
}

ECode WalkExportObject(
    /* [in] */ RPCType type,
    /* [out] */ String& strBuffer)
{
    HashMapCache<IObject*, IStub*>& registry = (type == RPCType::Local) ?
                                   sLocalExportRegistry : sRemoteExportRegistry;
    Mutex& registryLock = (type == RPCType::Local) ?
                           sLocalExportRegistryLock : sRemoteExportRegistryLock;

    strBuffer = "[";
    Mutex::AutoLock lock(registryLock);
    registry.GetValues(strBuffer, ExportRegistryWalker);

    if (strBuffer.GetByteLength() > 1) {
        // replace the last ',' with ']'
        char* str = (char*)strBuffer.string();
        str[strBuffer.GetByteLength() - 1] = ']';
    }
    else {
        strBuffer += "]";
    }

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

    strBuffer = "[";
    Mutex::AutoLock lock(registryLock);
    registry.GetValues(strBuffer, ImportRegistryWalker);

    if (strBuffer.GetByteLength() > 1) {
        // replace the last ',' with ']'
        char* str = (char*)strBuffer.string();
        str[strBuffer.GetByteLength() - 1] = ']';
    }
    else {
        strBuffer += "]";
    }

    return NOERROR;
}

} // namespace como
