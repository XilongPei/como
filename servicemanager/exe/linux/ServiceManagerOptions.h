//=========================================================================
// Copyright (C) 2023 The C++ Component Model(COMO) Open Source Project
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

#ifndef __JING_OPTIONS_H__
#define __JING_OPTIONS_H__

#include <string>
#include <comoapi.h>
#include <comosp.h>
#include <comoref.h>

namespace jing {

class ServiceManagerOptions
{
public:
    inline ServiceManagerOptions(
        /* [in] */ int argc,
        /* [in] */ char** argv);

    inline bool DoShowUsage() const;
    inline String GetPaxosServer() const;
    inline bool DoShowVersion() const;
    inline std::string GetLocalhost() const;
    inline std::string GetServiceManager() const;


    bool HasErrors() const;

    void ShowErrors() const;

    void ShowUsage() const;

    void ShowVersion() const;

private:
    void Parse(
        /* [in] */ int argc,
        /* [in] */ char** argv);

private:
    String mProgram;
    String mIllegalOptions;
    String mPaxosServer;

    std::string mLocalhost;
    std::string mServiceManager;

    bool mShowUsage = false;
    bool mShowVersion = false;
};

ServiceManagerOptions::ServiceManagerOptions(
    /* [in] */ int argc,
    /* [in] */ char** argv)
    : mPaxosServer(nullptr)
    , mLocalhost(std::string("tcp://127.0.0.1:8081"))
    , mServiceManager(std::string("tcp://127.0.0.1:8088"))
{
    Parse(argc, argv);
}

bool ServiceManagerOptions::DoShowUsage() const
{
    return mShowUsage;
}

bool ServiceManagerOptions::DoShowVersion() const
{
    return mShowVersion;
}

String ServiceManagerOptions::GetPaxosServer() const
{
    return mPaxosServer;
}

std::string ServiceManagerOptions::GetLocalhost() const
{
    return mLocalhost;
}

std::string ServiceManagerOptions::GetServiceManager() const
{
    return mServiceManager;
}

} // namespace jing

#endif // __JING_OPTIONS_H__
