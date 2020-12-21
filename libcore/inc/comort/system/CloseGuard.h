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

#ifndef __COMORT_SYSTEM_CLOSEGUARD_H__
#define __COMORT_SYSTEM_CLOSEGUARD_H__

#include "como/core/SyncObject.h"
#include "como.core.IStackTrace.h"
#include "comort.system.ICloseGuard.h"
#include "comort.system.ICloseGuardReporter.h"
#include "comort.system.ICloseGuardTracker.h"
#include <comosp.h>

using como::core::IStackTrace;
using como::core::SyncObject;

namespace comort {
namespace system {

class CloseGuard
    : public SyncObject
    , public ICloseGuard
{
private:
    class DefaultReporter
        : public SyncObject
        , public ICloseGuardReporter
    {
    public:
        COMO_INTERFACE_DECL();

        ECode Report(
            /* [in] */ const String& message,
            /* [in] */ IStackTrace* allocationSite) override;
    };

    class DefaultTracker
        : public SyncObject
        , public ICloseGuardTracker
    {
    public:
        COMO_INTERFACE_DECL();

        ECode Open(
            /* [in] */ IStackTrace* allocationSite) override;

        ECode Close(
            /* [in] */ IStackTrace* allocationSite) override;
    };

public:
    COMO_INTERFACE_DECL();

    static AutoPtr<ICloseGuard> Get();

    static ECode SetEnabled(
        /* [in] */ Boolean enabled);

    static Boolean IsEnabled();

    static ECode SetReporter(
        /* [in] */ ICloseGuardReporter* reporter);

    static AutoPtr<ICloseGuardReporter> GetReporter();

    static ECode SetTracker(
        /* [in] */ ICloseGuardTracker* tracker);

    static AutoPtr<ICloseGuardTracker> GetTracker();

    ECode Open(
        /* [in] */ const String& closer) override;

    ECode Close() override;

    ECode WarnIfOpen() override;

private:
    CloseGuard();

    static AutoPtr<ICloseGuard> GetNOOP();

    static AutoPtr<ICloseGuardReporter> GetOrSetREPORTER(
        /* [in] */ ICloseGuardReporter* reporter);

    static AutoPtr<ICloseGuardTracker> GetOrSetTRACKER(
        /* [in] */ ICloseGuardTracker* tracker);

private:
    static Boolean ENABLED;

    AutoPtr<IStackTrace> mAllocationSite;
};

inline CloseGuard::CloseGuard()
{}

inline ECode CloseGuard::SetEnabled(
    /* [in] */ Boolean enabled)
{
    ENABLED = enabled;
    return NOERROR;
}

inline Boolean CloseGuard::IsEnabled()
{
    return ENABLED;
}

inline AutoPtr<ICloseGuardReporter> CloseGuard::GetReporter()
{
    return GetOrSetREPORTER(nullptr);
}

inline AutoPtr<ICloseGuardTracker> CloseGuard::GetTracker()
{
    return GetOrSetTRACKER(nullptr);
}

}
}

#endif // __COMORT_SYSTEM_CLOSEGUARD_H__
