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

#ifndef __COMO_CORE_NATIVEFAULTHANDLER_H__
#define __COMO_CORE_NATIVEFAULTHANDLER_H__

#include <comotypes.h>
#include <signal.h>

namespace como {
namespace core {

class NativeFaultManager
{
public:
    NativeFaultManager();

    ~NativeFaultManager();

    void Init();

private:
    struct sigaction mOldaction;
    Boolean mInitialized;
};

class NativeFaultHandler
{
public:
    explicit NativeFaultHandler(
        /* [in] */ NativeFaultManager* manager);

    virtual ~NativeFaultHandler();

    NativeFaultManager* GetFaultManager();

    virtual Boolean Action(
        /* [in] */ int sig,
        /* [in] */ siginfo_t* siginfo,
        /* [in] */ void* context) = 0;

protected:
    NativeFaultManager* const mManager;
};

class StackOverflowHandler
    : public NativeFaultHandler
{
public:
    explicit StackOverflowHandler(
        /* [in] */ NativeFaultManager* manager);

    Boolean Action(
        /* [in] */ int sig,
        /* [in] */ siginfo_t* siginfo,
        /* [in] */ void* context) override;
};

extern NativeFaultManager sFaultManager;

}
}

#endif // __COMO_CORE_NATIVEFAULTHANDLER_H__
