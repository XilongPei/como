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

#ifndef __JING_SERVICEMANAGER_H__
#define __JING_SERVICEMANAGER_H__

#include <comoapi.h>
#include <comosp.h>
#include <comoref.h>
#include "hashmapCache.h"
#include "mutex.h"
#include <dbus/dbus.h>
#include "ServiceManagerOptions.h"

#ifdef COMO_FUNCTION_SAFETY
#include "ComoPhxUtils.h"
#endif

namespace jing {

class ServiceManager
    : public LightRefBase
{
public:

    //@ `InterfacePack`

    struct InterfacePack
    {
        ~InterfacePack()
        {
            ReleaseCoclassID(mCid);
            ReleaseInterfaceID(mIid);
        }

        String mServerName;
        String mDBusName;
        CoclassID mCid;
        InterfaceID mIid;
        Boolean mIsParcelable;
        Long mServerObjectId;
    };

public:
    static AutoPtr<ServiceManager> GetInstance();

    ECode AddService(
        /* [in] */ const String& name,
        /* [in] */ InterfacePack& object);

    ECode GetService(
        /* [in] */ const String& name,
        /* [out, callee] */ InterfacePack** object);

    ECode RemoveService(
        /* [in] */ const String& name);

    static DBusHandlerResult HandleMessage(
            /* [in] */ DBusConnection* conn,
            /* [in] */ DBusMessage* msg,
            /* [in] */ void* arg);

private:
    ServiceManager() {}
    ServiceManager(
        /* [in] */ ServiceManager&) {}

public:
    static constexpr const char* DBUS_NAME = "jing.servicemanager";
    static constexpr const char* OBJECT_PATH = "/jing/servicemanager";
    static constexpr const char* INTERFACE_PATH = "jing.servicemanager.IServiceManager";

private:
    static AutoPtr<ServiceManager> sInstance;

public:
    HashMapCache<String, InterfacePack*> mServices;
    Mutex mServicesLock;

    static ServiceManagerOptions *options;
#ifdef COMO_FUNCTION_SAFETY
    static como::PhxEchoServer *oEchoServer;
#endif
};

} // namespace jing

#endif // __JING_SERVICEMANAGER_H__
