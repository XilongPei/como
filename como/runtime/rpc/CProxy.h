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

//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
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

#ifndef __COMO_CPROXY_H__
#define __COMO_CPROXY_H__

#include "reflection/comoreflapi.h"
#include "type/comoarray.h"
#include "util/comosp.h"
#include "util/comoobj.h"
#include "util/MemPool.h"

namespace como {

class CProxy;

class InterfaceProxy
{
private:
    struct Registers
    {
        typedef Long Reg_t;

//
//----__aarch64__--------__aarch64__--------__aarch64__--------__aarch64__------
//
#if defined(__aarch64__)

        union IREG {
            Reg_t   reg;
            Long    lVal;
            Integer iVal;
        };

        union FPREG {
            Reg_t   reg;
            Double  dVal;
            Float   fVal;
        };

        IREG x0;
        IREG x1;
        IREG x2;
        IREG x3;
        IREG x4;
        IREG x5;
        IREG x6;
        IREG x7;
        IREG sp;

#if defined(ARM_FP_SUPPORT)
        FPREG d0;
        FPREG d1;
        FPREG d2;
        FPREG d3;
        FPREG d4;
        FPREG d5;
        FPREG d6;
        FPREG d7;
#endif

//
//----__arm__--------__arm__--------__arm__--------__arm__--------__arm__-------
//
#elif defined(__arm__)

        union IREG {
            Reg_t   reg;
            Long    lVal;
            Integer iVal;
        };

        union FPREG {
            Reg_t   reg;
            Double  dVal;
            Float   fVal;
        };

        IREG r0;
        IREG r1;
        IREG r2;
        IREG r3;
        IREG sp;

#if defined(ARM_FP_SUPPORT)
        FPREG d0;
        FPREG d1;
        FPREG d2;
        FPREG d3;
        FPREG d4;
        FPREG d5;
        FPREG d6;
        FPREG d7;
#endif

//
//----__x86_64__--------__x86_64__--------__x86_64__--------__x86_64__----------
//
#elif defined(__x86_64__)

        union GPReg {
            Reg_t   reg;
            Long    lVal;
            Integer iVal;
        };

        union SSEReg {
            Reg_t   reg;
            Double  dVal;
            Float   fVal;
        };

        GPReg rbp;
        GPReg rdi;
        GPReg rsi;
        GPReg rdx;
        GPReg rcx;
        GPReg r8;
        GPReg r9;

        SSEReg xmm0;
        SSEReg xmm1;
        SSEReg xmm2;
        SSEReg xmm3;
        SSEReg xmm4;
        SSEReg xmm5;
        SSEReg xmm6;
        SSEReg xmm7;

//
//----__i386__--------__i386__--------__i386__--------__i386__--------__i386__--
//
#elif defined(__i386__)
        union GPReg {
            Reg_t   reg;
            Long    lVal;
            Integer iVal;
        };

        union SSEReg {
            Reg_t   reg;
            Double  dVal;
            Float   fVal;
        };

        GPReg ebp;
        GPReg edi;
        GPReg esi;
        GPReg edx;
        GPReg ecx;

        SSEReg xmm0;
        SSEReg xmm1;
        SSEReg xmm2;
        SSEReg xmm3;
        SSEReg xmm4;
        SSEReg xmm5;
        SSEReg xmm6;
        SSEReg xmm7;

//
//----__riscv--__riscv_xlen == 64--------__riscv--__riscv_xlen == 64------------
//
#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

        union IREG {
            Reg_t   reg;
            Long    lVal;
            Integer iVal;
        };

        union FPREG {
            Reg_t   reg;
            Double  dVal;
            Float   fVal;
        };

        IREG x10;
        IREG x11;
        IREG x12;
        IREG x13;
        IREG x14;
        IREG x15;
        IREG x16;
        IREG x17;
        IREG sp;

        FPREG f10;
        FPREG f11;
        FPREG f12;
        FPREG f13;
        FPREG f14;
        FPREG f15;
        FPREG f16;
        FPREG f17;

        #endif
    #endif
#endif

        Integer paramStartOffset;
    };

public:
    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(Short id, const void *p);

    Integer AddRef(
        /* [in] */ HANDLE id = 0);

    Integer Release(
        /* [in] */ HANDLE id = 0);

    static Integer S_AddRef(
        /* [in] */ InterfaceProxy* thisObj,
        /* [in] */ HANDLE id);

    static Integer S_Release(
        /* [in] */ InterfaceProxy* thisObj,
        /* [in] */ HANDLE id);

    static IInterface* S_Probe(
        /* [in] */ InterfaceProxy* thisObj,
        /* [in] */ const InterfaceID& iid);

    static ECode S_GetInterfaceID(
        /* [in] */ InterfaceProxy* thisObj,
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID& iid);

    static ECode ProxyEntry(
        /* [in] */ HANDLE args);

private:
    ECode MarshalArguments(
        /* [in] */ Registers& regs,
        /* [in] */ IMetaMethod* method,
        /* [in] */ IParcel* argParcel);

    ECode UnmarshalResults(
        /* [in] */ Registers& regs,
        /* [in] */ IMetaMethod* method,
        /* [in] */ IParcel* resParcel);

    Integer GetIntegerValue(
        /* [in] */ Registers& regs,
        /* [in] */ Integer intParamIndex,
        /* [in] */ Integer fpParamIndex);

    Long GetLongValue(
        /* [in] */ Registers& regs,
        /* [in] */ Integer intParamIndex,
        /* [in] */ Integer fpParamIndex);

    Float GetFloatValue(
        /* [in] */ Registers& regs,
        /* [in] */ Integer intParamIndex,
        /* [in] */ Integer fpParamIndex);

    Double GetDoubleValue(
        /* [in] */ Registers& regs,
        /* [in] */ Integer intParamIndex,
        /* [in] */ Integer fpParamIndex);

private:
    friend class CProxy;

    static CMemPool *memPool;

    HANDLE*         mVtable;        // must be the first member
    HANDLE          mProxyEntry;    // must be the second member
    Integer         mIndex;         // Interface sequence number
    InterfaceID     mIid;
    IMetaInterface* mTargetMetadata;
    CProxy*         mOwner;
    Long            mTimeout;
};

extern const CoclassID CID_CProxy;

// HANDLE is a data structure more like "void *"
#define GetHANDLEValue GetLongValue

COCLASS_ID(228c4e6a-1df5-4130-b46e-d0322b676976)
class CProxy
    : public Object
    , public IProxy
{
public:
    CProxy();
    ~CProxy();

    COMO_OBJECT_DECL();

    COMO_INTERFACE_DECL();

    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(Short id, const void *p);

    void OnLastStrongRef(
        /* [in] */ const void* id) override;

    ECode GetTargetCoclass(
        /* [out] */ AutoPtr<IMetaCoclass>& target) override;

    ECode IsStubAlive(
        /* [in, out] */ Long& lvalue,
        /* [out] */ Boolean& alive) override;

    ECode ReleaseStub(
        /* [out] */ Boolean& alive) override;

    ECode ReleaseObject(
        /* [in] */ Long objectId) override;

    ECode LinkToDeath(
        /* [in] */ IDeathRecipient* recipient,
        /* [in] */ HANDLE cookie = 0,
        /* [in] */ Integer flags = 0) override;

    ECode UnlinkToDeath(
        /* [in] */ IDeathRecipient* recipient,
        /* [in] */ HANDLE cookie = 0,
        /* [in] */ Integer flags = 0,
        /* [out] */ AutoPtr<IDeathRecipient>* outRecipient = nullptr) override;

    ECode GetIpack(
        /* [out] */ AutoPtr<IInterfacePack>& ipack) override;

    ECode MonitorRuntime(
        /* [in] */ const Array<Byte>& request,
        /* [out] */ Array<Byte>& response) override;

    ECode GetUuidOrder(
        /* [out] */ Long& uuidOrder) override;

    ECode SetPubSocket(
        /* [in] */ HANDLE pubSocket) override;

    AutoPtr<IRPCChannel> GetChannel();

    CoclassID GetTargetCoclassID();

    static ECode CreateObject(
        /* [in] */ const CoclassID& cid,
        /* [in] */ IRPCChannel* channel,
        /* [in] */ IClassLoader* loader,
        /* [out] */ AutoPtr<IProxy>& proxy);

    // I don't want all members to be encapsulated
    AutoPtr<IInterfacePack> mIpack;
    String                  mServerName;
    Long                    mServerObjectId;

    // Member used in normal use phase, do not need to be passed between the
    // server and the client.
    // Order, A normal execution of a method.
    Long                    mUuidOrder;

private:
    friend class InterfaceProxy;

    static CMemPool*        memPool;

    CoclassID               mCid;
    IMetaCoclass*           mTargetMetadata;
    Array<InterfaceProxy*>  mInterfaces;
    AutoPtr<IRPCChannel>    mChannel;

    bool                    mMonitorInvokeMethod;
};

} // namespace como

#endif // __COMO_CPROXY_H__
