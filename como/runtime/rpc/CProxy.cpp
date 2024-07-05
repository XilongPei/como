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

#include "component/CBootClassLoader.h"
#include "rpc/comorpc.h"
#include "rpc/CProxy.h"
#include "ComoContext.h"
#include "util/comolog.h"
#include "reflection/reflHelpers.h"
#include "reflection/CMetaMethod.h"
#include <sys/mman.h>
#ifdef COMO_FUNCTION_SAFETY
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <ThreadPoolChannelInvoke.h>
#endif
#include "registry.h"
#include "RuntimeMonitor.h"
#include "mac.h"

namespace como {

#ifndef PAGE_SIZE
#define PAGE_SIZE (1u << 12)
#endif
#ifndef PAGE_MASK
#define PAGE_MASK (~(PAGE_SIZE - 1))
#endif
#ifndef PAGE_ALIGN
#define PAGE_ALIGN(va) (((va) + PAGE_SIZE - 1) & PAGE_MASK)
#endif

#if defined(__i386__) || defined(__arm__) || (defined(__riscv) && (__riscv_xlen == 32))
    #define GET_STACK_INTEGER(rbp, off, var)    \
        var = *(Integer *)(*(Integer *)&rbp + (int)off);
    #define GET_STACK_LONG(rbp, off, var)       \
        var = *(Long *)(*(Integer *)&rbp + (int)off);
    #define GET_STACK_FLOAT(rbp, off, var)      \
        var = *(Float *)(*(Integer *)&rbp + (int)off);
    #define GET_STACK_DOUBLE(rbp, off, var)     \
        var = *(Double *)(*(Integer *)&rbp + (int)off);
#elif defined(__x86_64__) || defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
    #define GET_STACK_INTEGER(rbp, off, var)    \
        var = *(Integer *)(*(Long *)&rbp + (int)off);
    #define GET_STACK_LONG(rbp, off, var)       \
        var = *(Long *)(*(Long *)&rbp + (int)off);
    #define GET_STACK_FLOAT(rbp, off, var)      \
        var = *(Float *)(*(Long *)&rbp + (int)off);
    #define GET_STACK_DOUBLE(rbp, off, var)     \
        var = *(Double *)(*(Long *)&rbp + (int)off);
#else
    #error Unknown Architecture
#endif


//
//----__aarch64__--------__aarch64__--------__aarch64__--------__aarch64__------
//
#if defined(__aarch64__)

#define GET_REG(reg, var)           \
    __asm__ __volatile__(           \
        "str   "#reg", %0;"         \
        : "=m"(var)                 \
    )

EXTERN_C void __entry();

__asm__(
    ".text;"
    ".align 8;"
    ".global __entry;"
    "__entry:"
    "sub    sp, sp, #32;"
    "stp    lr, x9, [sp, #16];"
    "mov    x9, #0x0;"
    "stp    x9, x0, [sp];"
    "ldr    x9, [x0, #8];"
    "mov    x0, sp;"
    "adr    lr, return_from_func;"
    "br     x9;"
    "return_from_func:"
    "ldp    lr, x9, [sp, #16];"
    "add    sp, sp, #32;"
    "ret;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
);
/*
0000000000400a00 <__entry>:
  400a00:       d10083ff        sub     sp, sp, #0x20
  400a04:       a90127fe        stp     x30, x9, [sp, #16]
  400a08:       d2800009        mov     x9, #0x0        # modify value #0x0 by statement: p[PROXY_INDEX_OFFSET] = i;
                                function in this source file, InterfaceProxy::ProxyEntry() {
                                    offset = 0;
                                    GET_STACK_INTEGER(args, offset, methodIndex);
                                }
                                the `methodIndex` correspond to ths `#0x0`
  400a0c:       a90003e9        stp     x9, x0, [sp]
  400a10:       f9400409        ldr     x9, [x0, #8]
  400a14:       910003e0        mov     x0, sp
                                aarch64 ABI: x0, the first parameter
  400a18:       1000005e        adr     x30, 400a20 <return_from_func>
  400a1c:       d61f0120        br      x9

0000000000400a20 <return_from_func>:
  400a20:       a94127fe        ldp     x30, x9, [sp, #16]
  400a24:       910083ff        add     sp, sp, #0x20
  400a28:       d65f03c0        ret
  400a2c:       d503201f        nop
  400a30:       d503201f        nop
  400a34:       d503201f        nop
  400a38:       d503201f        nop
  400a3c:       d503201f        nop
*/

//
//----__x86_64__--------__x86_64__--------__x86_64__--------__x86_64__----------
//
#elif defined(__x86_64__)

#define GET_REG(reg, var)           \
    __asm__ __volatile__(           \
        "movq   %%"#reg", %0;"      \
        : "=m"(var)                 \
    )

EXTERN_C void __entry();

__asm__(
    ".text;"
    ".align 8;"
    ".global __entry;"
    "__entry:"
    "pushq  %rbp;"
    "pushq  %rdi;"
    "subq   $8, %rsp;"
    "movl    $0xff, (%rsp);"
    "movq   %rdi, %rax;"
    "movq   %rsp, %rdi;"
    "call   *8(%rax);"
    "addq   $8, %rsp;"
    "popq   %rdi;"
    "popq   %rbp;"
    "ret;"
    "nop;"
    "nop;"
    "nop;"
);
/*
0000000000400848 <__entry>:
  400848:   55                      push   %rbp
  400849:   57                      push   %rdi
  40084a:   48 83 ec 08             sub    $0x8,%rsp
  40084e:   c7 04 24 ff 00 00 00    movl   $0xff,(%rsp)  # modify value ff by statement: p[PROXY_INDEX_OFFSET] = i;
                                function in this source file, InterfaceProxy::ProxyEntry() {
                                    offset = 0;
                                    GET_STACK_INTEGER(args, offset, methodIndex);
                                }
                                the `methodIndex` correspond to ths `$0xff`
  400855:   48 89 f8                mov    %rdi,%rax
  400858:   48 89 e7                mov    %rsp,%rdi
                                x64 ABI: %rdi, the first parameter
  40085b:   ff 50 08                callq  *0x8(%rax)
  40085e:   48 83 c4 08             add    $0x8,%rsp
  400862:   5f                      pop    %rdi
  400863:   5d                      pop    %rbp
  400864:   c3                      retq
  400865:   90                      nop
  400866:   90                      nop
  400867:   90                      nop
*/

//
//----__arm__--------__arm__--------__arm__--------__arm__--------__arm__-------
//
#elif defined(__arm__)

#define GET_REG(reg, var)           \
    __asm__ __volatile__(           \
        "str   "#reg", %0;"         \
        : "=m"(var)                 \
    )

EXTERN_C void __entry();

__asm__(
    ".text;"
    ".align 4;"
    ".global __entry;"
    "__entry:"
    "sub    sp, sp, #32;"

    //"stp    lr, x9, [sp, #16];"
    "add    sp, sp, #16;"
    "stmdb  sp!, {lr, r9};"

    "mov    r9, #0x0;"
    "stp    r9, r0, [sp];"
    "ldr    r9, [r0, #8];"
    "mov    r0, sp;"
    "adr    lr, return_from_func;"
    "br     r9;"
    "return_from_func:"

    //"ldp    lr, r9, [sp, #16];"
    "sub    sp, sp, #16;"
    "ldr    lr, [sp, #16];"

    "add    sp, sp, #32;"
    "ret;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
);

//
//----__i386__--------__i386__--------__i386__--------__i386__--------__i386__--
//
#elif defined(__i386__)

#define GET_REG(reg, var)           \
    __asm__ __volatile__(           \
        "mov   %%"#reg", %0;"       \
        : "=m"(var)                 \
    )

EXTERN_C void __entry();

__asm__(
    ".text;"
    ".align 4;"
    ".global __entry;"
    "__entry:"
    "endbr32;"
    "push  %ebp;"
    "push  %edi;"
    "sub   $4, %esp;"
    "movl  $0xff, (%esp);"
    "mov   %edi, %eax;"
    "mov   %esp, %edi;"
    "call  *4(%eax);"
    "add   $4, %esp;"
    "pop   %edi;"
    "pop   %ebp;"
    "ret;"
    "nop;"
    "nop;"
    "nop;"
);
/*
00001270 <__entry>:
    1270:   f3 0f 1e fb             endbr32
    1274:   55                      push   %ebp
    1275:   57                      push   %edi
    1276:   83 ec 04                sub    $0x4,%esp
    1279:   c7 04 24 ff 00 00 00    movl   $0xff,(%esp)  # modify value ff by statement: p[PROXY_INDEX_OFFSET] = i;
                                function in this source file, InterfaceProxy::ProxyEntry() {
                                    offset = 0;
                                    GET_STACK_INTEGER(args, offset, methodIndex);
                                }
                                the `methodIndex` correspond to ths `$0xff`
    1280:   89 f8                   mov    %edi,%eax
    1282:   89 e7                   mov    %esp,%edi
                                    x32 ABI: %edi, the first parameter
    1284:   ff 50 04                call   *0x4(%eax)
    1287:   83 c4 04                add    $0x4,%esp
    128a:   5f                      pop    %edi
    128b:   5d                      pop    %ebp
    128c:   c3                      ret
    128d:   90                      nop
    128e:   90                      nop
    128f:   90                      nop
*/


//
//----__riscv--__riscv_xlen == 64--------__riscv--__riscv_xlen == 64------------
//
#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

#define GET_REG(reg, var)           \
    __asm__ __volatile__(           \
        "ld   "#reg", %0;"          \
        : "=m"(var)                 \
    )

#define GET_F_REG(reg, var)         \
    __asm__ __volatile__(           \
        "fld   "#reg", %0;"         \
        : "=m"(var)                 \
    )

EXTERN_C void __entry();
__asm__(
    ".text;\n"
    ".align 8;\n"
    ".global __entry;\n"
    "__entry:\n"
    "addi   sp,sp,-40;\n"
    "sd     ra,32(sp);       # ra, Return address\n"
    "sd     s0,24(sp);       # ra, Return address\n"
    "addi   s0,sp,32;        # s0/fp， Saved register/frame pointer\n"

    "sd     a0,-24(s0);      # save double word\n"
    "ld     a5,-24(s0);      # \n"

    "addi   a5,a5,8;         # \n"
    "li     a4,0xff;         # modify value ff by statement: p[PROXY_INDEX_OFFSET] = i;\n"
    "              ;         #    function in this source file, InterfaceProxy::ProxyEntry() {\n"
    "              ;         #        offset = 0;\n"
    "              ;         #        GET_STACK_INTEGER(args, offset, methodIndex);\n"
    "              ;         #    }\n"
    "              ;         #    the `methodIndex` correspond to ths `$0xff`\n"
    "sw     a4,0(a5);        # save word"

    "mv     a0,a5;           # riscv64 ABI: x10/a0, the first parameter\n"
    "            ;           # ECode InterfaceProxy::ProxyEntry(HANDLE args)\n"

    "ld      a5,16(sp);      # load double word, restore s0\n"
    "jalr    a5;"

    "ld     s0,24(sp);       # load double word, restore s0\n"
    "ld     ra,32(sp);       # load double word, restore ra \n"
    "addi   sp,sp,40;        # \n"
    "ret;\n"
    "nop;\n"
);
/*
0000000000010700 <__entry>:
   10700:       fd810113                addi    sp,sp,-40
   10704:       02113023                sd      ra,32(sp)
   10708:       00813c23                sd      s0,24(sp)
   1070c:       02010413                addi    s0,sp,32
   10710:       fea43423                sd      a0,-24(s0)
   10714:       fe843783                ld      a5,-24(s0)
   10718:       00878793                addi    a5,a5,8
   1071c:       0ff00713                li      a4,255
   10720:       00e7a023                sw      a4,0(a5)
   10724:       01013783                ld      a5,16(sp)
   10728:       000780e7                jalr    a5
   1072c:       01813403                ld      s0,24(sp)
   10730:       02013083                ld      ra,32(sp)
   10734:       02810113                addi    sp,sp,40
   10738:       00008067                ret
   1073c:       00000013                nop
*/
        #endif
    #endif
#endif

HANDLE PROXY_ENTRY = 0;

#if defined(__aarch64__)
static constexpr Integer PROXY_ENTRY_SIZE = 64;
static constexpr Integer PROXY_ENTRY_SHIFT = 6;
static constexpr Integer PROXY_INDEX_OFFSET = 2;
#elif defined(__arm__)
static constexpr Integer PROXY_ENTRY_SIZE = 64;
static constexpr Integer PROXY_ENTRY_SHIFT = 6;
static constexpr Integer PROXY_INDEX_OFFSET = 2;
#elif defined(__x86_64__)
static constexpr Integer PROXY_ENTRY_SIZE = 32;
static constexpr Integer PROXY_ENTRY_SHIFT = 5;
static constexpr Integer PROXY_INDEX_OFFSET = 9;
#elif defined(__i386__)
static constexpr Integer PROXY_ENTRY_SIZE = 32;
static constexpr Integer PROXY_ENTRY_SHIFT = 5;
static constexpr Integer PROXY_INDEX_OFFSET = 13;
#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)
            static constexpr Integer PROXY_ENTRY_SIZE = 64;
            static constexpr Integer PROXY_ENTRY_SHIFT = 6;
            static constexpr Integer PROXY_INDEX_OFFSET = 28;
        #endif
    #endif
#endif
static constexpr Integer PROXY_ENTRY_NUMBER = 240;
static constexpr Integer METHOD_MAX_NUMBER = PROXY_ENTRY_NUMBER + 4;

/********
A COMO object can implemented at most METHOD_MAX_NUMBER Interfaces, COMO object
treats Interface as their inheritted class,
When call the Interface's method, C++ will do 'this pointer adjust'.

Map in memory:
+---                 sProxyVtable
|
|    ...         --> virtual table ...
|    Interface2  --> virtual table 3
|    Interface1  --> virtual table 2
|    IObject     --> virtual table 1
|COMO-Object

--------
‘this’ pointer in C++ https://www.geeksforgeeks.org/this-pointer-in-c/

To understand ‘this’ pointer, it is important to know how objects look at
functions and data members of a class.
Each object gets its own copy of the data member.
All-access the same function definition as present in the code segment.
Meaning each object gets its own copy of data members and all objects share a
single copy of member functions.
Then now question is that if only one copy of each member function exists and is
used by multiple objects, how are the proper data members are accessed and updated?
The compiler supplies an implicit pointer along with the names of the functions
as ‘this’.
The ‘this’ pointer is passed as a hidden argument to all nonstatic member
function calls and is available as a local variable within the body of all
nonstatic functions. ‘this’ pointer is not available in static member functions
as static member functions can be called without any object (with class name).
********/
static HANDLE sProxyVtable[METHOD_MAX_NUMBER];

void Init_Proxy_Entry()
{
    PROXY_ENTRY = reinterpret_cast<HANDLE>(mmap(nullptr,
            PAGE_ALIGN(PROXY_ENTRY_SIZE * PROXY_ENTRY_NUMBER),
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (PROXY_ENTRY == 0) {
        Logger::E("CProxy", "Mmap PROXY_ENTRY failed");
        return;
    }

#if defined(__aarch64__)
    Byte* p = (Byte*)PROXY_ENTRY;
    for (Integer i = 0;  i < PROXY_ENTRY_NUMBER;  i++) {
        memcpy(p, reinterpret_cast<void*>(&__entry), PROXY_ENTRY_SIZE);
        Integer* codes = reinterpret_cast<Integer*>(p);
        codes[PROXY_INDEX_OFFSET] = codes[PROXY_INDEX_OFFSET] | (i << 5);
        p += PROXY_ENTRY_SIZE;
    }
#elif defined(__arm__)
    Byte* p = (Byte*)PROXY_ENTRY;
    for (Integer i = 0;  i < PROXY_ENTRY_NUMBER;  i++) {
        memcpy(p, reinterpret_cast<void*>(&__entry), PROXY_ENTRY_SIZE);
        Integer* codes = reinterpret_cast<Integer*>(p);
        codes[PROXY_INDEX_OFFSET] = codes[PROXY_INDEX_OFFSET] | (i << 5);
        p += PROXY_ENTRY_SIZE;
    }
#elif defined(__x86_64__)
    Byte* p = (Byte*)PROXY_ENTRY;
    for (Integer i = 0;  i < PROXY_ENTRY_NUMBER;  i++) {
        memcpy(p, reinterpret_cast<void*>(&__entry), PROXY_ENTRY_SIZE);
        p[PROXY_INDEX_OFFSET] = i;
        p += PROXY_ENTRY_SIZE;
    }
#elif defined(__i386__)
    Byte* p = (Byte*)PROXY_ENTRY;
    for (Integer i = 0;  i < PROXY_ENTRY_NUMBER;  i++) {
        memcpy(p, reinterpret_cast<void*>(&__entry), PROXY_ENTRY_SIZE);
        p[PROXY_INDEX_OFFSET] = i;
        p += PROXY_ENTRY_SIZE;
    }
#elif defined(__riscv)
    #if (__riscv_xlen == 64)
        Byte* p = (Byte*)PROXY_ENTRY;
        for (Integer i = 0;  i < PROXY_ENTRY_NUMBER;  i++) {
            memcpy(p, reinterpret_cast<void*>(&__entry), PROXY_ENTRY_SIZE);
            Integer* codes = reinterpret_cast<Integer*>(p);
            codes[PROXY_INDEX_OFFSET] = codes[PROXY_INDEX_OFFSET] | (i << 5);
            p += PROXY_ENTRY_SIZE;
        }
    #endif
#endif

    sProxyVtable[0] = reinterpret_cast<HANDLE>(&InterfaceProxy::S_AddRef);
    sProxyVtable[1] = reinterpret_cast<HANDLE>(&InterfaceProxy::S_Release);
    sProxyVtable[2] = reinterpret_cast<HANDLE>(&InterfaceProxy::S_Probe);
    sProxyVtable[3] = reinterpret_cast<HANDLE>(&InterfaceProxy::S_GetInterfaceID);
    for (Integer i = 4;  i < METHOD_MAX_NUMBER;  i++) {
        sProxyVtable[i] = PROXY_ENTRY + ((i - 4) << PROXY_ENTRY_SHIFT);
    }
}

void Uninit_Proxy_Entry()
{
    if (PROXY_ENTRY != 0) {
        munmap(reinterpret_cast<void*>(PROXY_ENTRY),
                PAGE_ALIGN(PROXY_ENTRY_SIZE * PROXY_ENTRY_NUMBER));
        PROXY_ENTRY = 0;
    }
}

//----------------------------------------------------------------------------------

Integer InterfaceProxy::AddRef(
    /* [in] */ HANDLE id)
{
    return 1;
}

Integer InterfaceProxy::Release(
    /* [in] */ HANDLE)
{
    return 1;
}

Integer InterfaceProxy::S_AddRef(
    /* [in] */ InterfaceProxy* thisObj,
    /* [in] */ HANDLE id)
{
    return thisObj->mOwner->AddRef(id);
}

Integer InterfaceProxy::S_Release(
    /* [in] */ InterfaceProxy* thisObj,
    /* [in] */ HANDLE id)
{
    return thisObj->mOwner->Release(id);
}

IInterface* InterfaceProxy::S_Probe(
    /* [in] */ InterfaceProxy* thisObj,
    /* [in] */ const InterfaceID& iid)
{
    return thisObj->mOwner->Probe(iid);
}

ECode InterfaceProxy::S_GetInterfaceID(
    /* [in] */ InterfaceProxy* thisObj,
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID& iid)
{
    return thisObj->mOwner->GetInterfaceID(object, iid);
}

ECode InterfaceProxy::MarshalArguments(
    /* [in] */ Registers& regs,
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel)
{
    Integer intParamIndex = 1;
    Integer fpParamIndex = 0;
    CMetaMethod* cMethod = CMetaMethod::From(method);
    Integer N = cMethod->mParameters.GetLength();
    if (N > 0) {
        // Help me to call BuildAllParameters once.
        Integer outArgs;
        method->GetOutArgumentsNumber(outArgs);
    }
    else {
        return NOERROR;
    }

    for (Integer i = 0;  i < N;  i++) {
        IMetaParameter* param = cMethod->mParameters[i];

        AutoPtr<IMetaType> type;
        IOAttribute ioAttr;
        TypeKind kind;

        param->GetType(type);
        param->GetIOAttribute(ioAttr);
        type->GetTypeKind(kind);

        if (ioAttr == IOAttribute::IN) {
            switch (kind) {
                case TypeKind::Char: {
                    Char value = (Char)GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteChar(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Byte: {
                    Byte value = (Byte)GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteByte(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Short: {
                    Short value = (Short)GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteShort(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Integer: {
                    Integer value = GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteInteger(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Long: {
                    Long value = GetLongValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteLong(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Float: {
                    Float value = GetFloatValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteFloat(value));
                    fpParamIndex++;
                    break;
                }
                case TypeKind::Double: {
                    Double value = GetDoubleValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteDouble(value));
                    fpParamIndex++;
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean value = (Boolean)GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteBoolean(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::String: {
                    String value = *reinterpret_cast<String*>(GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteString(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::ECode: {
                    ECode value = (ECode)GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteECode(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Enum: {
                    Integer value = GetIntegerValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteEnumeration(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }
                    if (elemKind == TypeKind::CoclassID ||
                            elemKind == TypeKind::ComponentID ||
                            elemKind == TypeKind::InterfaceID ||
                            elemKind == TypeKind::HANDLE) {
                        Logger::E("CProxy", "Invalid [in] Array(%s), param index: %d",
                                            TypeKindHelper::GetTypeName(elemKind), i);
                        return E_ILLEGAL_ARGUMENT_EXCEPTION;
                    }

                    HANDLE value = GetHANDLEValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteArray(*reinterpret_cast<Triple*>(value)));

                    intParamIndex++;
                    break;
                }
                case TypeKind::Interface: {
                    IInterface* value = reinterpret_cast<IInterface*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteInterface(value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [in] type(%s), param index: %d",
                                            TypeKindHelper::GetTypeName(kind), i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::IN_OUT) {
            switch (kind) {
                case TypeKind::Char: {
                    Char* addr = reinterpret_cast<Char*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteChar(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Byte: {
                    Byte* addr = reinterpret_cast<Byte*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteByte(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Short: {
                    Short* addr = reinterpret_cast<Short*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteShort(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Integer: {
                    Integer* addr = reinterpret_cast<Integer*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteInteger(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Long: {
                    Long* addr = reinterpret_cast<Long*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteLong(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Float: {
                    Float* addr = reinterpret_cast<Float*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    argParcel->WriteFloat(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Double: {
                    Double* addr = reinterpret_cast<Double*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteDouble(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean* addr = reinterpret_cast<Boolean*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteBoolean(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::String: {
                    String* addr = reinterpret_cast<String*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteString(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::ECode: {
                    ECode* addr = reinterpret_cast<ECode*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteECode(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Enum: {
                    Integer* addr = reinterpret_cast<Integer*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteInteger(*addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }
                    if (elemKind == TypeKind::CoclassID ||
                            elemKind == TypeKind::ComponentID ||
                            elemKind == TypeKind::InterfaceID ||
                            elemKind == TypeKind::HANDLE) {
                        Logger::E("CProxy", "Invalid [in, out] Array(%d), param index: %d", elemKind, i);
                        return E_ILLEGAL_ARGUMENT_EXCEPTION;
                    }

                    HANDLE value = GetHANDLEValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteArray(*reinterpret_cast<Triple*>(value)));

                    intParamIndex++;
                    break;
                }
                case TypeKind::Interface: {
                    IInterface** value = reinterpret_cast<IInterface**>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    FAIL_RETURN(argParcel->WriteInterface(*value));
                    intParamIndex++;
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [in, out] type(%d), param index: %d", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT) {
            switch (kind) {
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Float:
                case TypeKind::Double:
                case TypeKind::Boolean:
                case TypeKind::String:
                case TypeKind::ECode:
                case TypeKind::Enum:
                case TypeKind::Interface:
                    intParamIndex++;
                    break;
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }
                    if (elemKind == TypeKind::CoclassID ||
                            elemKind == TypeKind::ComponentID ||
                            elemKind == TypeKind::InterfaceID ||
                            elemKind == TypeKind::HANDLE) {
                        Logger::E("CProxy", "Invalid [out] Array(%d), param index: %d", elemKind, i);
                        return E_ILLEGAL_ARGUMENT_EXCEPTION;
                    }

                    HANDLE value = GetHANDLEValue(regs, intParamIndex, fpParamIndex);
                    FAIL_RETURN(argParcel->WriteArray(*reinterpret_cast<Triple*>(value)));

                    intParamIndex++;
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [out] type(%d), param index: %d", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT_CALLEE) {
            switch (kind) {
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }
                    if (elemKind == TypeKind::CoclassID ||
                            elemKind == TypeKind::ComponentID ||
                            elemKind == TypeKind::InterfaceID ||
                            elemKind == TypeKind::HANDLE) {
                        Logger::E("CProxy", "Invalid [out, callee] Array(%d), param index: %d", elemKind, i);
                        return E_ILLEGAL_ARGUMENT_EXCEPTION;
                    }
                    intParamIndex++;
                    break;
                }
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Float:
                case TypeKind::Double:
                case TypeKind::Boolean:
                case TypeKind::String:
                case TypeKind::ECode:
                case TypeKind::Enum:
                case TypeKind::Interface:
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [out, callee] type(%d), param index: %d", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
    }

    return NOERROR;
}

ECode InterfaceProxy::UnmarshalResults(
    /* [in] */ Registers& regs,
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* resParcel)
{
    CMetaMethod* cMethod = CMetaMethod::From(method);
    Integer N = cMethod->mParameters.GetLength();
    if (N > 0) {
        // Help me to call BuildAllParameters once.
        Integer outArgs;
        method->GetOutArgumentsNumber(outArgs);
        if (outArgs <= 0) {
            return NOERROR;
        }
    }
    else
        return NOERROR;

    // `8 * 2` corresponds to the two statements in CStub.cpp
    // The data in parcel is 8-byte aligned
    //
    // resParcel->WriteInteger(RPC_MAGIC_NUMBER);
    // resParcel->WriteLong(uuid64);
    //
    resParcel->SetDataPosition(8 * 2);

    Long hash;
    mOwner->GetHashCode(hash);
    resParcel->SetProxyInfo(hash);
    resParcel->SetServerInfo(mOwner->mServerName);

    Integer intParamIndex = 1, fpParamIndex = 0;

    for (Integer i = 0;  i < N;  i++) {
        IMetaParameter* param = cMethod->mParameters[i];

        AutoPtr<IMetaType> type;
        IOAttribute ioAttr;
        TypeKind kind;

        param->GetType(type);
        param->GetIOAttribute(ioAttr);
        type->GetTypeKind(kind);

        if (ioAttr == IOAttribute::IN) {
            switch (kind) {
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Boolean:
                case TypeKind::String:
                case TypeKind::ECode:
                case TypeKind::Enum:
                case TypeKind::Array:
                case TypeKind::Interface:
                    intParamIndex++;
                    break;
                case TypeKind::Float:
                case TypeKind::Double:
                    fpParamIndex++;
                    break;
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [in] type(%s), param index: %d",
                                            TypeKindHelper::GetTypeName(kind), i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if ((ioAttr == IOAttribute::OUT) || (ioAttr == IOAttribute::IN_OUT)) {
            switch (kind) {
                case TypeKind::Char: {
                    Char* addr = reinterpret_cast<Char*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadChar(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Byte: {
                    Byte* addr = reinterpret_cast<Byte*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadByte(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Short: {
                    Short* addr = reinterpret_cast<Short*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadShort(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Integer: {
                    Integer* addr = reinterpret_cast<Integer*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadInteger(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Long: {
                    Long* addr = reinterpret_cast<Long*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadLong(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Float: {
                    Float* addr = reinterpret_cast<Float*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadFloat(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Double: {
                    Double* addr = reinterpret_cast<Double*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadDouble(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean* addr = reinterpret_cast<Boolean*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadBoolean(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::String: {
                    String* addr = reinterpret_cast<String*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadString(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::ECode: {
                    ECode* addr = reinterpret_cast<ECode*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadECode(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Enum: {
                    Integer* addr = reinterpret_cast<Integer*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadEnumeration(*addr);
                    intParamIndex++;
                    break;
                }
                case TypeKind::Array: {
                    HANDLE addr = GetHANDLEValue(regs, intParamIndex, fpParamIndex);
                    resParcel->ReadArray(reinterpret_cast<Triple*>(addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Interface: {
                    AutoPtr<IInterface>* intf = reinterpret_cast<AutoPtr<IInterface>*>(
                            GetHANDLEValue(regs, intParamIndex, fpParamIndex));
                    resParcel->ReadInterface(*intf);
                    intParamIndex++;
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [in, out] or [out] type(%d), param index: %d", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT_CALLEE) {
            switch (kind) {
                case TypeKind::Array: {
                    HANDLE addr = GetHANDLEValue(regs, intParamIndex, fpParamIndex);
                    resParcel->ReadArray(reinterpret_cast<Triple*>(addr));
                    intParamIndex++;
                    break;
                }
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Float:
                case TypeKind::Double:
                case TypeKind::Boolean:
                case TypeKind::String:
                case TypeKind::ECode:
                case TypeKind::Enum:
                case TypeKind::Interface:
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::Triple:
                default:
                    Logger::E("CProxy", "Invalid [out, callee] type(%d), param index: %d", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
    }

    return NOERROR;
}

Integer InterfaceProxy::GetIntegerValue(
    /* [in] */ Registers& regs,
    /* [in] */ Integer intParamIndex,
    /* [in] */ Integer fpParamIndex)
{
#if defined(__aarch64__)

    switch (intParamIndex) {
        case 0:
            return regs.x0.iVal;
        case 1:
            return regs.x1.iVal;
        case 2:
            return regs.x2.iVal;
        case 3:
            return regs.x3.iVal;
        case 4:
            return regs.x4.iVal;
        case 5:
            return regs.x5.iVal;
        case 6:
            return regs.x6.iVal;
        case 7:
            return regs.x7.iVal;
        default: {
            Integer value, offset;
#if defined(ARM_FP_SUPPORT)
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 8
                       : intParamIndex - 8 + fpParamIndex - 8
                     );
#else
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 8
                       : intParamIndex - 8 + fpParamIndex
                     );
#endif
            offset += regs.paramStartOffset;
            offset *= 8;
            GET_STACK_INTEGER(regs.sp, offset, value);
            return value;
        }
    }

#elif defined(__arm__)

    switch (intParamIndex) {
        case 0:
            return regs.r0.iVal;
        case 1:
            return regs.r1.iVal;
        case 2:
            return regs.r2.iVal;
        case 3:
            return regs.r3.iVal;
        default: {
            Integer value, offset;
#if defined(ARM_FP_SUPPORT)
            offset = ( (fpParamIndex <= 3)
                       ? intParamIndex - 4
                       : intParamIndex - 4 + fpParamIndex - 8
                     );
#else
            offset = ( (fpParamIndex <= 3)
                       ? intParamIndex - 4
                       : intParamIndex - 4 + fpParamIndex
                     );
#endif
            offset += regs.paramStartOffset;
            offset *= 4;
            GET_STACK_INTEGER(regs.sp, offset, value);
            return value;
        }
    }

#elif defined(__x86_64__)

    switch (intParamIndex) {
        case 0:
            return regs.rdi.iVal;
        case 1:
            return regs.rsi.iVal;
        case 2:
            return regs.rdx.iVal;
        case 3:
            return regs.rcx.iVal;
        case 4:
            return regs.r8.iVal;
        case 5:
            return regs.r9.iVal;
        default: {
            Integer value, offset;
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 6
                       : intParamIndex - 6 + fpParamIndex - 8
                     );
            offset += regs.paramStartOffset;
            offset *= 8;
            GET_STACK_INTEGER(regs.rbp, offset, value);
            return value;
        }
    }

#elif defined(__i386__)

#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

    switch (intParamIndex) {
        case 0:
            return regs.x10.iVal;
        case 1:
            return regs.x11.iVal;
        case 2:
            return regs.x12.iVal;
        case 3:
            return regs.x13.iVal;
        case 4:
            return regs.x14.iVal;
        case 5:
            return regs.x15.iVal;
        case 6:
            return regs.x16.iVal;
        case 7:
            return regs.x17.iVal;
        default: {
            Integer value, offset;
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 8
                       : intParamIndex - 8 + fpParamIndex - 8
                     );
            offset += regs.paramStartOffset;
            offset *= 8;
            GET_STACK_INTEGER(regs.sp, offset, value);
            return value;
        }
    }

        #endif
    #endif
#endif
}

Long InterfaceProxy::GetLongValue(
    /* [in] */ Registers& regs,
    /* [in] */ Integer intParamIndex,
    /* [in] */ Integer fpParamIndex)
{
#if defined(__aarch64__)

    switch (intParamIndex) {
        case 0:
            return regs.x0.lVal;
        case 1:
            return regs.x1.lVal;
        case 2:
            return regs.x2.lVal;
        case 3:
            return regs.x3.lVal;
        case 4:
            return regs.x4.lVal;
        case 5:
            return regs.x5.lVal;
        case 6:
            return regs.x6.lVal;
        case 7:
            return regs.x7.lVal;
        default: {
            Integer offset;
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 8
                       : intParamIndex - 8 + fpParamIndex - 8
                     );
            offset += regs.paramStartOffset;
            offset *= 8;
            Long value;
            GET_STACK_LONG(regs.sp, offset, value);
            return value;
        }
    }

#elif defined(__arm__)

#elif defined(__x86_64__)

    switch (intParamIndex) {
        case 0:
            return regs.rdi.lVal;
        case 1:
            return regs.rsi.lVal;
        case 2:
            return regs.rdx.lVal;
        case 3:
            return regs.rcx.lVal;
        case 4:
            return regs.r8.lVal;
        case 5:
            return regs.r9.lVal;
        default: {
            Integer offset;
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 6
                       : intParamIndex - 6 + fpParamIndex - 8
                     );
            offset += regs.paramStartOffset;
            offset *= 8;
            Long value;
            GET_STACK_LONG(regs.rbp, offset, value);
            return value;
        }
    }

#elif defined(__i386__)

#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

    switch (intParamIndex) {
        case 0:
            return regs.x10.lVal;
        case 1:
            return regs.x11.lVal;
        case 2:
            return regs.x12.lVal;
        case 3:
            return regs.x13.lVal;
        case 4:
            return regs.x14.lVal;
        case 5:
            return regs.x15.lVal;
        case 6:
            return regs.x16.lVal;
        case 7:
            return regs.x17.lVal;
        default: {
            Integer offset;
            offset = ( (fpParamIndex <= 7)
                       ? intParamIndex - 8
                       : intParamIndex - 8 + fpParamIndex - 8
                     );
            offset += regs.paramStartOffset;
            offset *= 8;
            Long value;
            GET_STACK_LONG(regs.sp, offset, value);
            return value;
        }
    }

        #endif
    #endif

#endif
}

Float InterfaceProxy::GetFloatValue(
    /* [in] */ Registers& regs,
    /* [in] */ Integer intParamIndex,
    /* [in] */ Integer fpParamIndex)
{
#if defined(__aarch64__)

#if defined(ARM_FP_SUPPORT)
    switch (fpParamIndex) {
        case 0:
            return regs.d0.fVal;
        case 1:
            return regs.d1.fVal;
        case 2:
            return regs.d2.fVal;
        case 3:
            return regs.d3.fVal;
        case 4:
            return regs.d4.fVal;
        case 5:
            return regs.d5.fVal;
        case 6:
            return regs.d6.fVal;
        case 7:
            return regs.d7.fVal;
        default: {
            Integer offset = ( (intParamIndex <= 7)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 8
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Float value;
            GET_STACK_FLOAT(regs.sp, offset, value);
            return value;
        }
    }
#else
    Integer offset = ( (intParamIndex <= 7)
                       ? fpParamIndex
                       : fpParamIndex + intParamIndex - 8
                     );
    offset += regs.paramStartOffset;
    offset *= 8;
    Float value;
    GET_STACK_FLOAT(regs.sp, offset, value);
    return value;
#endif

#elif defined(__x86_64__)

    switch (fpParamIndex) {
        case 0:
            return regs.xmm0.fVal;
        case 1:
            return regs.xmm1.fVal;
        case 2:
            return regs.xmm2.fVal;
        case 3:
            return regs.xmm3.fVal;
        case 4:
            return regs.xmm4.fVal;
        case 5:
            return regs.xmm5.fVal;
        case 6:
            return regs.xmm6.fVal;
        case 7:
            return regs.xmm7.fVal;
        default: {
            Integer offset = ( (intParamIndex <= 5)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 6
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Float value;
            GET_STACK_FLOAT(regs.rbp, offset, value);
            return value;
        }
    }

#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

    switch (fpParamIndex) {
        case 0:
            return regs.f10.fVal;
        case 1:
            return regs.f11.fVal;
        case 2:
            return regs.f12.fVal;
        case 3:
            return regs.f13.fVal;
        case 4:
            return regs.f14.fVal;
        case 5:
            return regs.f15.fVal;
        case 6:
            return regs.f16.fVal;
        case 7:
            return regs.f17.fVal;
        default: {
            Integer offset = ( (intParamIndex <= 7)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 8
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Float value;
            GET_STACK_FLOAT(regs.sp, offset, value);
            return value;
        }
    }

        #endif
    #endif

#endif
}

Double InterfaceProxy::GetDoubleValue(
    /* [in] */ Registers& regs,
    /* [in] */ Integer intParamIndex,
    /* [in] */ Integer fpParamIndex)
{
#if defined(__aarch64__)

#if defined(ARM_FP_SUPPORT)
    switch (fpParamIndex) {
        case 0:
            return regs.d0.dVal;
        case 1:
            return regs.d1.dVal;
        case 2:
            return regs.d2.dVal;
        case 3:
            return regs.d3.dVal;
        case 4:
            return regs.d4.dVal;
        case 5:
            return regs.d5.dVal;
        case 6:
            return regs.d6.dVal;
        case 7:
            return regs.d7.dVal;
        default: {
            Integer offset = ( (intParamIndex <= 7)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 8
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Double value;
            GET_STACK_DOUBLE(regs.sp, offset, value);
            return value;
        }
    }
#else
    Integer offset = ( (intParamIndex <= 7)
                       ? fpParamIndex
                       : fpParamIndex + intParamIndex - 8
                     );
    offset += regs.paramStartOffset;
    offset *= 8;
    Double value;
    GET_STACK_DOUBLE(regs.sp, offset, value);
    return value;
#endif

#elif defined(__arm__)

#elif defined(__x86_64__)

    switch (fpParamIndex) {
        case 0:
            return regs.xmm0.dVal;
        case 1:
            return regs.xmm1.dVal;
        case 2:
            return regs.xmm2.dVal;
        case 3:
            return regs.xmm3.dVal;
        case 4:
            return regs.xmm4.dVal;
        case 5:
            return regs.xmm5.dVal;
        case 6:
            return regs.xmm6.dVal;
        case 7:
            return regs.xmm7.dVal;
        default: {
            Integer offset = ( (intParamIndex <= 5)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 6
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Double value;
            GET_STACK_DOUBLE(regs.rbp, offset, value);
            return value;
        }
    }

#elif defined(__i386__)

#else
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

    switch (fpParamIndex) {
        case 0:
            return regs.f10.dVal;
        case 1:
            return regs.f11.dVal;
        case 2:
            return regs.f12.dVal;
        case 3:
            return regs.f13.dVal;
        case 4:
            return regs.f14.dVal;
        case 5:
            return regs.f15.dVal;
        case 6:
            return regs.f16.dVal;
        case 7:
            return regs.f17.dVal;
        default: {
            Integer offset = ( (intParamIndex <= 7)
                               ? fpParamIndex - 8
                               : fpParamIndex - 8 + intParamIndex - 8
                             );
            offset += regs.paramStartOffset;
            offset *= 8;
            Double value;
            GET_STACK_DOUBLE(regs.sp, offset, value);
            return value;
        }
    }

        #endif
    #endif

#endif
}

ECode InterfaceProxy::ProxyEntry(
    /* [in] */ HANDLE args)
{
    // collect registers information should be done at the beginning of function
    Registers regs;

//----__aarch64__--------__aarch64__--------__aarch64__--------__aarch64__------
#if defined(__aarch64__)
    GET_REG(x0, regs.x0.reg);
    GET_REG(x1, regs.x1.reg);
    GET_REG(x2, regs.x2.reg);
    GET_REG(x3, regs.x3.reg);
    GET_REG(x4, regs.x4.reg);
    GET_REG(x5, regs.x5.reg);
    GET_REG(x6, regs.x6.reg);
    GET_REG(x7, regs.x7.reg);

#if defined(ARM_FP_SUPPORT)
    GET_REG(d0, regs.d0.reg);
    GET_REG(d1, regs.d1.reg);
    GET_REG(d2, regs.d2.reg);
    GET_REG(d3, regs.d3.reg);
    GET_REG(d4, regs.d4.reg);
    GET_REG(d5, regs.d5.reg);
    GET_REG(d6, regs.d6.reg);
    GET_REG(d7, regs.d7.reg);
#endif

    regs.sp.reg = args + 32;
    regs.paramStartOffset = 0;

//----__arm__--------__arm__--------__arm__--------__arm__--------__arm__-------
#elif defined(__arm__)
    GET_REG(x0, regs.r0.reg);
    GET_REG(x1, regs.r1.reg);
    GET_REG(x2, regs.r2.reg);
    GET_REG(x3, regs.r3.reg);

#if defined(ARM_FP_SUPPORT)
    GET_REG(d0, regs.d0.reg);
    GET_REG(d1, regs.d1.reg);
    GET_REG(d2, regs.d2.reg);
    GET_REG(d3, regs.d3.reg);
    GET_REG(d4, regs.d4.reg);
    GET_REG(d5, regs.d5.reg);
    GET_REG(d6, regs.d6.reg);
    GET_REG(d7, regs.d7.reg);
#endif

    regs.sp.reg = args + 32;
    regs.paramStartOffset = 0;

//----__x86_64__--------__x86_64__--------__x86_64__--------__x86_64__----------
#elif defined(__x86_64__)
    GET_REG(rdi, regs.rdi.reg);
    GET_REG(rsi, regs.rsi.reg);
    GET_REG(rdx, regs.rdx.reg);
    GET_REG(rcx, regs.rcx.reg);
    GET_REG(r8, regs.r8.reg);
    GET_REG(r9, regs.r9.reg);

    GET_REG(xmm0, regs.xmm0.reg);
    GET_REG(xmm1, regs.xmm1.reg);
    GET_REG(xmm2, regs.xmm2.reg);
    GET_REG(xmm3, regs.xmm3.reg);
    GET_REG(xmm4, regs.xmm4.reg);
    GET_REG(xmm5, regs.xmm5.reg);
    GET_REG(xmm6, regs.xmm6.reg);
    GET_REG(xmm7, regs.xmm7.reg);

    regs.rbp.reg = args + 16;
    regs.paramStartOffset = 2;

//----__i386__--------__i386__--------__i386__--------__i386__--------__i386__--
#elif defined(__i386__)

#else
//----__riscv--__riscv_xlen == 64--------__riscv--__riscv_xlen == 64------------
    #if defined(__riscv)
        #if (__riscv_xlen == 64)

    regs.sp.reg = args + 32;
    regs.paramStartOffset = 0;
    GET_REG(x10, regs.x10.reg);
    GET_REG(x11, regs.x11.reg);
    GET_REG(x12, regs.x12.reg);
    GET_REG(x13, regs.x13.reg);
    GET_REG(x14, regs.x14.reg);
    GET_REG(x15, regs.x15.reg);
    GET_REG(x16, regs.x16.reg);
    GET_REG(x17, regs.x17.reg);

    GET_F_REG(f10, regs.f10.reg);
    GET_F_REG(f11, regs.f11.reg);
    GET_F_REG(f12, regs.f12.reg);
    GET_F_REG(f13, regs.f13.reg);
    GET_F_REG(f14, regs.f14.reg);
    GET_F_REG(f15, regs.f15.reg);
    GET_F_REG(f16, regs.f16.reg);
    GET_F_REG(f17, regs.f17.reg);

        #endif
    #endif

#endif

    Integer methodIndex;
    InterfaceProxy *thisObj;
    Integer offset;

    GET_STACK_INTEGER(args, 0, methodIndex);

#if defined(__i386__) || defined(__arm__) || (defined(__riscv) && (__riscv_xlen == 32))
    offset = sizeof(Integer);
#elif defined(__x86_64__) || defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
    offset = sizeof(Long);
#else
    #error Unknown Architecture
#endif
    GET_STACK_LONG(args, offset, *(Long *)&thisObj);

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String name, ns;
        // It's just operation about reference count, No error checking required
        (void)thisObj->mTargetMetadata->GetName(name);
        (void)thisObj->mTargetMetadata->GetNamespace(ns);
        Logger::D("CProxy", "Call ProxyEntry with interface \"%s::%s\"",
                                                    ns.string(), name.string());
    }

    AutoPtr<IMetaMethod> method;
    thisObj->mTargetMetadata->GetMethod(methodIndex + 4, method);

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String name, signature;
        (void)method->GetName(name);
        (void)method->GetSignature(signature);
        Logger::D("CProxy", "Call ProxyEntry with method \"%s(%s)\"",
                                             name.string(), signature.string());
    }

    ECode ec;
    AutoPtr<IParcel> inParcel, outParcel;
    RPCType type;

    thisObj->mOwner->mChannel->GetRPCType(type);
    ec = CoCreateParcel(type, inParcel);
    if (FAILED(ec)) {
        goto ProxyExit;
    }

    ec = inParcel->WriteInteger(RPC_MAGIC_NUMBER);
    ec |= inParcel->WriteInteger(thisObj->mIndex);    // interfaceIndex
    ec |= inParcel->WriteInteger(methodIndex + 4);
    if (FAILED(ec)) {
        goto ProxyExit;
    }

    /**
     * In distributed computing, nodes do not synchronize time. Lamport clock is
     * used to define the sequence of events.
     */
    Long uuid64;
#if 0
    ec = inParcel->WriteLong(Mac::GetUuid64(uuid64));
    if (FAILED(ec)) {
        goto ProxyExit;
    }
#else
    uuid64 = ComoContext::gComoContext->gLamportClock->send_event();
    ec = inParcel->WriteLong(Mac::CompoundUuid64(uuid64));
    if (FAILED(ec)) {
        goto ProxyExit;
    }
#endif

    ec = thisObj->MarshalArguments(regs, method, inParcel);
    if (FAILED(ec)) {
        goto ProxyExit;
    }

    thisObj->mOwner->mUuidOrder = uuid64;

    if (thisObj->mOwner->mMonitorInvokeMethod) {
        RuntimeMonitor::WriteRtmInvokeMethod(uuid64,
                                         thisObj->mOwner->mServerObjectId,
                                         thisObj->mOwner->mCid, thisObj->mIid,
                                         RTM_ParamTransDirection::CALL_METHOD,
                                         methodIndex + 4, inParcel,
                                         RTM_WhichQueue::InvokeMethodClientQueue);
    }

    if (! thisObj->mOwner->mServerName.IsEmpty()) {
        // remote process call
        ec = thisObj->mOwner->mChannel->Invoke(method, inParcel, outParcel);
        if (SUCCEEDED(ec)) {
            if (thisObj->mOwner->mMonitorInvokeMethod) {
                Integer magicParcel;
                Long uuid64Parcel;

                outParcel->ReadInteger(magicParcel);
                outParcel->ReadLong(uuid64Parcel);
                outParcel->SetDataPosition(0);

                if ((magicParcel != RPC_MAGIC_NUMBER) || (uuid64Parcel != uuid64)) {
                    Logger::E("CProxy", "Bad result packet: Magic Number: 0x%x, ID: %lld",
                                                                magicParcel, uuid64Parcel);
                }

                RuntimeMonitor::WriteRtmInvokeMethod(uuid64,
                                    thisObj->mOwner->mServerObjectId,
                                    thisObj->mOwner->mCid, thisObj->mIid,
                                    RTM_ParamTransDirection::RETURN_FROM_METHOD,
                                    methodIndex + 4, outParcel,
                                    RTM_WhichQueue::InvokeMethodClientQueue);
            }

            ec = thisObj->UnmarshalResults(regs, method, outParcel);
        }
        goto ProxyExit;
    }

#ifndef COMO_FUNCTION_SAFETY
    ec = thisObj->mOwner->mChannel->Invoke(method, inParcel, outParcel);
    if (FAILED(ec)) {
        goto ProxyExit;
    }
#else
    if (0 == thisObj->mTimeout) {
        ec = thisObj->mOwner->mChannel->Invoke(method, inParcel, outParcel);
        if (FAILED(ec)) {
            goto ProxyExit;
        }
    }
    else {
        struct timespec curTime;
        clock_gettime(CLOCK_REALTIME, &curTime);

        // the unit of mTimeout is microsecond [Millisecond, microsecond, nanosecond]
        Long nsec = curTime.tv_nsec + (thisObj->mTimeout % 1000000) * 1000;
                                                          //123456     123
        curTime.tv_sec = curTime.tv_sec + nsec / 1000000000 + thisObj->mTimeout / 1000000;
                                                //123456789                      //123456
        curTime.tv_nsec = nsec % 1000000000;
                                //123456789

        int pos = TPCI_Executor::GetInstance()->RunTask(thisObj->mOwner->mChannel,
                                                     method, inParcel, outParcel);
        if (pos < 0) {
            ec = FUNCTION_SAFETY_CALL_OUT_OF_MEMORY;
            goto ProxyExit;
        }
        int ret = pthread_cond_timedwait(
                            &(ThreadPoolChannelInvoke::mWorkerList[pos]->mCond),
                            &(ThreadPoolChannelInvoke::mWorkerList[pos]->mMutex),
                            &curTime);
        if (ret != ETIMEDOUT /*110, time out*/) {
            ec = ThreadPoolChannelInvoke::mWorkerList[pos]->ec;
        }
        else {
            ec = FUNCTION_SAFETY_CALL_TIMEOUT;
        }

        TPCI_Executor::GetInstance()->CleanTask(pos);

        if (FAILED(ec)) {
            goto ProxyExit;
        }
    }
#endif

    if (thisObj->mOwner->mMonitorInvokeMethod) {
        Integer magicParcel;
        Long uuid64Parcel;

        outParcel->ReadInteger(magicParcel);
        outParcel->ReadLong(uuid64Parcel);
        outParcel->SetDataPosition(0);

        if ((magicParcel != RPC_MAGIC_NUMBER) || (uuid64Parcel != uuid64)) {
            Logger::E("CProxy", "Bad result packet: Magic Number: 0x%x, ID: %lld",
                                                        magicParcel, uuid64Parcel);
        }

        RuntimeMonitor::WriteRtmInvokeMethod(uuid64,
                            thisObj->mOwner->mServerObjectId,
                            thisObj->mOwner->mCid, thisObj->mIid,
                            RTM_ParamTransDirection::RETURN_FROM_METHOD,
                            methodIndex + 4, outParcel,
                            RTM_WhichQueue::InvokeMethodClientQueue);
    }

    ec = thisObj->UnmarshalResults(regs, method, outParcel);

ProxyExit:
    Logger::D("CProxy", "Exit ProxyEntry with ec(0x%X)", ec);

    return ec;
}

//----------------------------------------------------------------------

const CoclassID CID_CProxy =
        {{0x228c4e6a,0x1df5,0x4130,0xb46e,{0xd0,0x32,0x2b,0x67,0x69,0x76}}, &CID_COMORuntime};

COMO_OBJECT_IMPL(CProxy);

CProxy::CProxy()
    : mUuidOrder(0)
{}

CProxy::~CProxy()
{
    for (Integer i = 0;  i < mInterfaces.GetLength();  i++) {
        InterfaceProxy* iproxy = mInterfaces[i];
        mInterfaces[i] = nullptr;

#ifdef COMO_FUNCTION_SAFETY_RTOS
        InterfaceProxy::MemPoolFree(0, iproxy);
#else
        delete iproxy;
#endif
    }
}

Integer CProxy::AddRef(
    /* [in] */ HANDLE id)
{
    return Object::AddRef(id);
}

Integer CProxy::Release(
    /* [in] */ HANDLE id)
{
    return Object::Release(id);
}

IInterface* CProxy::Probe(
    /* [in] */ const InterfaceID& iid)
{
    if (IID_IInterface == iid) {
        return (IObject*)this;
    }
    else if (IID_IObject == iid) {
        return (IObject*)this;
    }
    else if (IID_IProxy == iid) {
        return (IProxy*)this;
    }
    for (Integer i = 0;  i < mInterfaces.GetLength();  i++) {
        InterfaceProxy* iproxy = mInterfaces[i];
        if (iproxy->mIid == iid) {
            return reinterpret_cast<IInterface*>(&iproxy->mVtable);
        }
    }
    return nullptr;
}

ECode CProxy::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID& iid)
{
    if (object == (IObject*)this) {
        iid = IID_IObject;
        return NOERROR;
    }
    for (Integer i = 0; i < mInterfaces.GetLength(); i++) {
        InterfaceProxy* iproxy = mInterfaces[i];
        if ((IInterface*)iproxy == object) {
            iid = iproxy->mIid;
            return NOERROR;
        }
    }
    return E_ILLEGAL_ARGUMENT_EXCEPTION;
}

ECode CProxy::GetTargetCoclass(
    /* [out] */ AutoPtr<IMetaCoclass>& target)
{
    target = mTargetMetadata;
    return NOERROR;
}

ECode CProxy::IsStubAlive(
    /* [in, out] */ Long& lvalue,
    /* [out] */ Boolean& alive)
{
    return mChannel->IsPeerAlive(lvalue, alive);
}

ECode CProxy::ReleaseStub(
    /* [out] */ Boolean& alive)
{
    return mChannel->ReleasePeer(alive);
}

ECode CProxy::ReleaseObject(
    /* [in] */ Long objectId)
{
    return mChannel->ReleaseObject(objectId);
}

ECode CProxy::LinkToDeath(
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags)
{
    return mChannel->LinkToDeath(this, recipient, cookie, flags);
}

ECode CProxy::UnlinkToDeath(
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags,
    /* [out] */ AutoPtr<IDeathRecipient>* outRecipient)
{
    return mChannel->UnlinkToDeath(this, recipient, cookie, flags, outRecipient);
}

ECode CProxy::GetIpack(
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    ipack = mIpack;
    return NOERROR;
}

ECode CProxy::MonitorRuntime(
    /* [in] */ const Array<Byte>& request,
    /* [out] */ Array<Byte>& response)
{
    if (request.GetLength() < sizeof(RTM_Command)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    RTM_Command *rtmCommand = (RTM_Command *)request.GetPayload();
    switch (rtmCommand->command) {
        case RTM_CommandType::CMD_Client_Activate_InvokeMethod: {
            mMonitorInvokeMethod = true;
            return NOERROR;
        }

        case RTM_CommandType::CMD_Client_Deactivate_InvokeMethod: {
            mMonitorInvokeMethod = false;
            return NOERROR;
        }

        case RTM_CommandType::CMD_Client_InvokeMethod: {
            Mutex::AutoLock lock(RuntimeMonitor::rtmInvokeMethodClientQueue_Lock);

            if (lwrb_get_free(RuntimeMonitor::rtmLwRB_ClientQueue) != 0) {
                size_t len;
                if (lwrb_peek(RuntimeMonitor::rtmLwRB_ClientQueue, 0, &len,
                                                        sizeof(size_t)) != sizeof(size_t)) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                response = Array<Byte>(len);
                if (! response.IsNull()) {
                    if (lwrb_read(RuntimeMonitor::rtmLwRB_ClientQueue,
                                                      response.GetPayload(), len) != len) {
                        return E_OUT_OF_MEMORY_ERROR;
                    }
                }
            }
            return NOERROR;
        }

        case RTM_CommandType::CMD_Client_Dump_InvokeMethod: {
            Mutex::AutoLock lock(RuntimeMonitor::rtmInvokeMethodClientQueue_Lock);

            String strBuffer = "[";

            while(lwrb_get_free(RuntimeMonitor::rtmLwRB_ClientQueue) != 0) {
                size_t len;
                if (lwrb_peek(RuntimeMonitor::rtmLwRB_ClientQueue, 0, &len,
                                                        sizeof(size_t)) != sizeof(size_t)) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                Array<Byte> buffer = Array<Byte>(len);
                if (! buffer.IsNull()) {
                    if (lwrb_read(RuntimeMonitor::rtmLwRB_ClientQueue,
                                                        buffer.GetPayload(), len) != len) {
                        return E_OUT_OF_MEMORY_ERROR;
                    }
                }

                RuntimeMonitor::DeserializeRtmInvokeMethod(
                                reinterpret_cast<como::RTM_InvokeMethod*>(buffer.GetPayload()));

                String str;
                ECode ec = RuntimeMonitor::DumpRtmInvokeMethod(
                           reinterpret_cast<como::RTM_InvokeMethod*>(buffer.GetPayload()), str);
                if (SUCCEEDED(ec))
                    strBuffer += (str + ",");
                else
                    break;
            }

            if (strBuffer.GetByteLength() > 1) {
                // replace the last ',' with ']'
                char* str = (char*)strBuffer.string();
                str[strBuffer.GetByteLength() - 1] = ']';
            }
            else {
                strBuffer += "]";
            }

            response = Array<Byte>(strBuffer.GetByteLength()+1);
            if (! response.IsNull()) {
                response.CopyRaw((Byte*)strBuffer.string(), strBuffer.GetByteLength()+1);
            }
            return NOERROR;
        }

        // server side command: CMD_BY_STRING, CMD_Server_Activate_InvokeMethod, CMD_Server_Deactivate_InvokeMethod
        default:
            ;
    }

    // Monitoring command cannot be handled by client, send it to the server.
    return mChannel->MonitorRuntime(request, response);
}

AutoPtr<IRPCChannel> CProxy::GetChannel()
{
    return mChannel;
}

CoclassID CProxy::GetTargetCoclassID()
{
    return mCid;
}

ECode CProxy::CreateObject(
    /* [in] */ const CoclassID& cid,
    /* [in] */ IRPCChannel* channel,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IProxy>& proxy)
{
    proxy = nullptr;

    if (nullptr == loader) {
        loader = CBootClassLoader::GetSystemClassLoader();
    }

    /**
     * First load the coclass from local cache (CBootClassLoader::mComponents).
     * If the loading is unsuccessful, obtain the component metadata from the
     * server side.
     */
    AutoPtr<IMetaCoclass> mc;
    ECode ec = loader->LoadCoclass(cid, mc);
    if (FAILED(ec)) {
        Array<Byte> metadata;
        ec = channel->GetComponentMetadata(cid, metadata);
        if (FAILED(ec)) {
            Logger::E("CProxy", "GetComponentMetadata failed. ECode: 0x%X", ec);
            return E_CLASS_NOT_FOUND_EXCEPTION;
        }

        AutoPtr<IMetaComponent> component;
        ec = loader->LoadMetadata(metadata, component);
        if (FAILED(ec)) {
            Logger::E("CProxy", "LoadMetadata failed. ECode: 0x%X", ec);
            return E_CLASS_NOT_FOUND_EXCEPTION;
        }

        ec = component->GetCoclass(cid, mc);
        if (FAILED(ec)) {
            Logger::E("CProxy", "Get IMetaCoclass failed. ECode: 0x%X", ec);
            return E_CLASS_NOT_FOUND_EXCEPTION;
        }
    }

#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = MemPoolAlloc(sizeof(CProxy));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    CProxy *cProxy = new(buf) CProxy();
    AutoPtr<CProxy> proxyObj = cProxy;
    cProxy->SetFunFreeMem(MemPoolFree, 0);
#else
    AutoPtr<CProxy> proxyObj = new CProxy();
    if (nullptr == proxyObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    mc->GetCoclassID(proxyObj->mCid);
    proxyObj->mTargetMetadata = mc;
    proxyObj->mChannel = channel;
    proxyObj->mIpack = nullptr;

    Long lvalue;
    ec = reflHelpers::coclassGetConstantLong(mc, String("monitor"), lvalue);
    if (FAILED(ec)) {
        // If it is an error such as the definition cannot be found, it is normal
        if (E_TYPE_MISMATCH_EXCEPTION == ec) {
            Logger::E("CProxy", "Wrong monitor datatype");
        }
        proxyObj->mMonitorInvokeMethod = false;
    }
    else {
        proxyObj->mMonitorInvokeMethod = true;
    }

    Integer interfaceNumber;
    mc->GetInterfaceNumber(interfaceNumber);
    Array<IMetaInterface*> interfaces(interfaceNumber);
    if (interfaces.IsNull()) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    mc->GetAllInterfaces(interfaces);
    proxyObj->mInterfaces = Array<InterfaceProxy*>(interfaceNumber);
    if (proxyObj->mInterfaces.IsNull()) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    channel->GetServerName(proxyObj->mServerName);
    channel->GetServerObjectId(proxyObj->mServerObjectId);

    for (Integer i = 0; i < interfaceNumber; i++) {
#ifdef COMO_FUNCTION_SAFETY_RTOS
        void *buf = MemPoolAlloc(sizeof(InterfaceProxy));
        if (nullptr == buf) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        InterfaceProxy *interfaceProxy = new(buf) InterfaceProxy();
        AutoPtr<InterfaceProxy> iproxy = interfaceProxy;
#else
        AutoPtr<InterfaceProxy> iproxy = new InterfaceProxy();
        if (nullptr == iproxy) {
            return E_OUT_OF_MEMORY_ERROR;
        }
#endif

        iproxy->mIndex = i;
        iproxy->mOwner = proxyObj;
        iproxy->mTargetMetadata = interfaces[i];
        iproxy->mTargetMetadata->GetInterfaceID(iproxy->mIid);
        iproxy->mVtable = sProxyVtable;
        iproxy->mProxyEntry = reinterpret_cast<HANDLE>(&InterfaceProxy::ProxyEntry);
        proxyObj->mInterfaces[i] = iproxy;
#ifdef COMO_FUNCTION_SAFETY
        ec = reflHelpers::intfGetConstantLong(iproxy->mTargetMetadata,
                                           String("timeout"), iproxy->mTimeout);
        if (FAILED(ec)) {
            if (E_TYPE_MISMATCH_EXCEPTION == ec) {
                Logger::E("CProxy", "Wrong timeout datatype");
            }
            iproxy->mTimeout = 0;
        }
#endif
    }

    proxy = proxyObj;
    return NOERROR;
}

ECode CProxy::GetUuidOrder(
    /* [out] */ Long& uuidOrder)
{
    uuidOrder = mUuidOrder;
    return NOERROR;
}

ECode CProxy::SetPubSocket(
    /* [in] */ HANDLE pubSocket)
{
    mChannel->SetPubSocket(pubSocket);
    return NOERROR;
}

void CProxy::OnLastStrongRef(
    /* [in] */ const void* id)
{
    /**
     * Lower level programs should not call higher-level logic.
     * OnLastStrongRef is called by Object (Object::OnLastStrongRef)
     */
#if 0
    if (nullptr == mIpack)
        return;

    Long proxyId;
    mIpack->GetProxyInfo(proxyId);
    UnregisterImportObjectByChannel(RPCType::Local, proxyId);
#endif

    Object::OnLastStrongRef(id);
}

CMemPool *CProxy::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_Proxy,
                                         sizeof(CProxy));
void *CProxy::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void CProxy::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

CMemPool *InterfaceProxy::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_InterfaceProxy,
                                                 sizeof(InterfaceProxy));
void *InterfaceProxy::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void InterfaceProxy::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

} // namespace como
