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

#ifndef __COMO_CCMDEF_H__
#define __COMO_CCMDEF_H__

#if defined(_DEBUG)
#include <assert.h>
#endif
#include <cstddef>
#include <cstdint>

namespace como {

#define COCLASS_ID(x)
#define INTERFACE_ID(x)

#define interface       struct

#define Coclass(name)   class name : public _##name

#ifndef EXTERN_C
#define EXTERN_C        extern "C"
#endif

#define COM_PUBLIC      __attribute__ ((visibility ("default")))
#define COM_LOCAL       __attribute__ ((visibility ("hidden")))

#define INIT_PROI_1     __attribute__ ((init_priority (500)))
#define INIT_PROI_2     __attribute__ ((init_priority (1000)))
#define INIT_PROI_3     __attribute__ ((init_priority (1500)))
#define INIT_PROI_4     __attribute__ ((init_priority (2000)))
#define INIT_PROI_5     __attribute__ ((init_priority (2500)))
#define INIT_PROI_6     __attribute__ ((init_priority (3000)))
#define INIT_PROI_7     __attribute__ ((init_priority (3500)))
#define INIT_PROI_8     __attribute__ ((init_priority (4000)))
#define INIT_PROI_9     __attribute__ ((init_priority (4500)))
#define INIT_PROI_10    __attribute__ ((init_priority (5000)))

#define CONS_PROI_1     __attribute__ ((constructor (500)))
#define CONS_PROI_2     __attribute__ ((constructor (1000)))
#define CONS_PROI_3     __attribute__ ((constructor (1500)))
#define CONS_PROI_4     __attribute__ ((constructor (2000)))
#define CONS_PROI_5     __attribute__ ((constructor (2500)))
#define CONS_PROI_6     __attribute__ ((constructor (3000)))
#define CONS_PROI_7     __attribute__ ((constructor (3500)))
#define CONS_PROI_8     __attribute__ ((constructor (4000)))
#define CONS_PROI_9     __attribute__ ((constructor (4500)))
#define CONS_PROI_10    __attribute__ ((constructor (5000)))

#define DEST_PROI_1     __attribute__ ((destructor (500)))
#define DEST_PROI_2     __attribute__ ((destructor (1000)))
#define DEST_PROI_3     __attribute__ ((destructor (1500)))
#define DEST_PROI_4     __attribute__ ((destructor (2000)))
#define DEST_PROI_5     __attribute__ ((destructor (2500)))
#define DEST_PROI_6     __attribute__ ((destructor (3000)))
#define DEST_PROI_7     __attribute__ ((destructor (3500)))
#define DEST_PROI_8     __attribute__ ((destructor (4000)))
#define DEST_PROI_9     __attribute__ ((destructor (4500)))
#define DEST_PROI_10    __attribute__ ((destructor (5000)))

#define REFCOUNT_ADD(i)     if (i) { (i)->AddRef(); }
#define REFCOUNT_RELEASE(i) if (i) { (i)->Release(); }

#define VALIDATE_NOT_NULL(i) \
        if (i == nullptr) { return E_ILLEGAL_ARGUMENT_EXCEPTION; }

#ifndef FAIL_RETURN
#define FAIL_RETURN(expr)           \
    do {                            \
        ECode ec = expr;            \
        if (FAILED(ec)) return ec;  \
    } while(0);
#endif

#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif

#define ALIGN4(v)       (((v) + 3) & ~3)
#define ALIGN8(v)       (((v) + 7) & ~7)

#if defined(__i386__) || defined(__arm__)
#define ALIGN(v)        ALIGN4(v)
#elif defined(__x86_64__) || defined(__aarch64__)
#define ALIGN(v)        ALIGN8(v)
#endif

template<typename T>
constexpr T RoundDown(T x, size_t n)
{
    return x & ~(n - 1);
}

template<typename T>
constexpr T RoundUp(T x, size_t n)
{
    return RoundDown(x + n - 1, n);
}

template<typename T>
inline T* AlignDown(T* x, uintptr_t n)
{
    return reinterpret_cast<T*>(RoundDown(reinterpret_cast<uintptr_t>(x), n));
}

template<typename T, int N>
inline int ArrayLength(const T (&)[N])
{
    return N;
}

#ifndef CHECK
#if defined(_DEBUG)
#define CHECK(e)        assert(e)
#define BLOCK_CHECK()   if (true)
#else
#define CHECK(e)
#define BLOCK_CHECK()   if (false)
#endif
#endif

#ifndef PACKED
#define PACKED(x)       __attribute__ ((__aligned__(x), __packed__))
#endif

#ifdef __GNUC__
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)       (x)
#define UNLIKELY(x)     (x)
#endif

#ifndef COMO_INTERFACE_DECL
#define COMO_INTERFACE_DECL()                           \
    Integer AddRef(                                     \
        /* [in] */ HANDLE id = 0) override;             \
                                                        \
    Integer Release(                                    \
        /* [in] */ HANDLE id = 0) override;             \
                                                        \
    IInterface* Probe(                                  \
        /* [in] */ const InterfaceID& iid) override;    \
                                                        \
    ECode GetInterfaceID(                               \
        /* [in] */ IInterface* object,                  \
        /* [out] */ InterfaceID& iid) override;
#endif

#ifndef COMO_INTERFACE_IMPL_1
#define COMO_INTERFACE_IMPL_1(ClassName, SuperclassName, InterfaceName)       \
    Integer ClassName::AddRef(                             \
        /* [in] */ HANDLE id)                              \
    {                                                      \
        return Object::AddRef(id);                         \
    }                                                      \
                                                           \
    Integer ClassName::Release(                            \
        /* [in] */ HANDLE id)                              \
    {                                                      \
        return Object::Release(id);                        \
    }                                                      \
                                                           \
    IInterface* ClassName::Probe(                          \
        /* [in] */ const InterfaceID& iid)                 \
    {                                                      \
        if (iid == IID_IInterface) {                       \
            return (IInterface*)(InterfaceName*)this;      \
        }                                                  \
        else if (iid == IID_##InterfaceName) {             \
            return (InterfaceName*)this;                   \
        }                                                  \
        return SuperclassName::Probe(iid);                 \
    }                                                      \
                                                           \
    ECode ClassName::GetInterfaceID(                       \
        /* [in] */ IInterface* object,                     \
        /* [out] */ InterfaceID& iid)                      \
    {                                                      \
        if (object == (IInterface*)(InterfaceName*)this) { \
            iid = IID_##InterfaceName;                     \
            return NOERROR;                                \
        }                                                  \
        return SuperclassName::GetInterfaceID(object, iid); \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_2
#define COMO_INTERFACE_IMPL_2(ClassName, SuperclassName, Interface1, Interface2) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_3
#define COMO_INTERFACE_IMPL_3(ClassName, SuperclassName, Interface1, Interface2, Interface3) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface3) {                     \
            return (Interface3*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface3*)this) {    \
            iid = IID_##Interface3;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_4
#define COMO_INTERFACE_IMPL_4(ClassName, SuperclassName, Interface1, Interface2, Interface3, Interface4) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface3) {                     \
            return (Interface3*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface4) {                     \
            return (Interface4*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface3*)this) {    \
            iid = IID_##Interface3;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface4*)this) {    \
            iid = IID_##Interface4;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_5
#define COMO_INTERFACE_IMPL_5(ClassName, SuperclassName, Interface1, Interface2, Interface3, Interface4, Interface5) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface3) {                     \
            return (Interface3*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface4) {                     \
            return (Interface4*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface5) {                     \
            return (Interface5*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface3*)this) {    \
            iid = IID_##Interface3;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface4*)this) {    \
            iid = IID_##Interface4;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface5*)this) {    \
            iid = IID_##Interface5;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_6
#define COMO_INTERFACE_IMPL_6(ClassName, SuperclassName, Interface1, Interface2, Interface3, Interface4, Interface5, Interface6) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface3) {                     \
            return (Interface3*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface4) {                     \
            return (Interface4*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface5) {                     \
            return (Interface5*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface6) {                     \
            return (Interface6*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface3*)this) {    \
            iid = IID_##Interface3;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface4*)this) {    \
            iid = IID_##Interface4;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface5*)this) {    \
            iid = IID_##Interface5;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface6*)this) {    \
            iid = IID_##Interface6;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_7
#define COMO_INTERFACE_IMPL_7(ClassName, SuperclassName, Interface1, Interface2, Interface3, Interface4, Interface5, Interface6, Interface7) \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }                                                           \
                                                                \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(Interface1*)this;              \
        }                                                       \
        else if (iid == IID_##Interface1) {                     \
            return (Interface1*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface2) {                     \
            return (Interface2*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface3) {                     \
            return (Interface3*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface4) {                     \
            return (Interface4*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface5) {                     \
            return (Interface5*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface6) {                     \
            return (Interface6*)this;                           \
        }                                                       \
        else if (iid == IID_##Interface7) {                     \
            return (Interface7*)this;                           \
        }                                                       \
        return SuperclassName::Probe(iid);                      \
    }                                                           \
                                                                \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(Interface1*)this) {         \
            iid = IID_##Interface1;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface2*)this) {    \
            iid = IID_##Interface2;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface3*)this) {    \
            iid = IID_##Interface3;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface4*)this) {    \
            iid = IID_##Interface4;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface5*)this) {    \
            iid = IID_##Interface5;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface6*)this) {    \
            iid = IID_##Interface6;                             \
        }                                                       \
        else if (object == (IInterface*)(Interface7*)this) {    \
            iid = IID_##Interface7;                             \
        }                                                       \
        else {                                                  \
            return SuperclassName::GetInterfaceID(object, iid); \
        }                                                       \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_LIGHT_1
#define COMO_INTERFACE_IMPL_LIGHT_1(ClassName, SuperclassName, InterfaceName) \
    Integer ClassName::AddRef(                             \
        /* [in] */ HANDLE id)                              \
    {                                                      \
        return LightRefBase::AddRef(id);                   \
    }                                                      \
                                                           \
    Integer ClassName::Release(                            \
        /* [in] */ HANDLE id)                              \
    {                                                      \
        return LightRefBase::Release(id);                  \
    }                                                      \
                                                           \
    IInterface* ClassName::Probe(                          \
        /* [in] */ const InterfaceID& iid)                 \
    {                                                      \
        if (iid == IID_IInterface) {                       \
            return (IInterface*)(InterfaceName*)this;      \
        }                                                  \
        else if (iid == IID_##InterfaceName) {             \
            return (InterfaceName*)this;                   \
        }                                                  \
        return SuperclassName::Probe(iid);                 \
    }                                                      \
                                                           \
    ECode ClassName::GetInterfaceID(                       \
        /* [in] */ IInterface* object,                     \
        /* [out] */ InterfaceID& iid)                      \
    {                                                      \
        if (object == (IInterface*)(InterfaceName*)this) { \
            iid = IID_##InterfaceName;                     \
            return NOERROR;                                \
        }                                                  \
        return SuperclassName::GetInterfaceID(object, iid); \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_LIGHT_2
#define COMO_INTERFACE_IMPL_LIGHT_2(ClassName, SuperclassName, InterfaceName1, InterfaceName2) \
    Integer ClassName::AddRef(                              \
        /* [in] */ HANDLE id)                               \
    {                                                       \
        return LightRefBase::AddRef(id);                    \
    }                                                       \
                                                            \
    Integer ClassName::Release(                             \
        /* [in] */ HANDLE id)                               \
    {                                                       \
        return LightRefBase::Release(id);                   \
    }                                                       \
                                                            \
    IInterface* ClassName::Probe(                           \
        /* [in] */ const InterfaceID& iid)                  \
    {                                                       \
        if (iid == IID_IInterface) {                        \
            return (IInterface*)(InterfaceName1*)this;      \
        }                                                   \
        else if (iid == IID_##InterfaceName1) {             \
            return (InterfaceName1*)this;                   \
        }                                                   \
        else if (iid == IID_##InterfaceName2) {             \
            return (InterfaceName2*)this;                   \
        }                                                   \
        return SuperclassName::Probe(iid);                  \
    }                                                       \
                                                            \
    ECode ClassName::GetInterfaceID(                        \
        /* [in] */ IInterface* object,                      \
        /* [out] */ InterfaceID& iid)                       \
    {                                                       \
        if (object == (IInterface*)(InterfaceName1*)this) { \
            iid = IID_##InterfaceName1;                     \
            return NOERROR;                                 \
        }                                                   \
        else if (object == (IInterface*)(InterfaceName2*)this) {    \
            iid = IID_##InterfaceName2;                     \
            return NOERROR;                                 \
        }                                                   \
        return SuperclassName::GetInterfaceID(object, iid); \
    }
#endif

#ifndef COMO_INTERFACE_IMPL_LIGHT_3
#define COMO_INTERFACE_IMPL_LIGHT_3(ClassName, SuperclassName, InterfaceName1, InterfaceName2, InterfaceName3) \
    Integer ClassName::AddRef(                              \
        /* [in] */ HANDLE id)                               \
    {                                                       \
        return LightRefBase::AddRef(id);                    \
    }                                                       \
                                                            \
    Integer ClassName::Release(                             \
        /* [in] */ HANDLE id)                               \
    {                                                       \
        return LightRefBase::Release(id);                   \
    }                                                       \
                                                            \
    IInterface* ClassName::Probe(                           \
        /* [in] */ const InterfaceID& iid)                  \
    {                                                       \
        if (iid == IID_IInterface) {                        \
            return (IInterface*)(InterfaceName1*)this;      \
        }                                                   \
        else if (iid == IID_##InterfaceName1) {             \
            return (InterfaceName1*)this;                   \
        }                                                   \
        else if (iid == IID_##InterfaceName2) {             \
            return (InterfaceName2*)this;                   \
        }                                                   \
        else if (iid == IID_##InterfaceName3) {             \
            return (InterfaceName3*)this;                   \
        }                                                   \
        return SuperclassName::Probe(iid);                  \
    }                                                       \
                                                            \
    ECode ClassName::GetInterfaceID(                        \
        /* [in] */ IInterface* object,                      \
        /* [out] */ InterfaceID& iid)                       \
    {                                                       \
        if (object == (IInterface*)(InterfaceName1*)this) { \
            iid = IID_##InterfaceName1;                     \
            return NOERROR;                                 \
        }                                                   \
        else if (object == (IInterface*)(InterfaceName2*)this) {    \
            iid = IID_##InterfaceName2;                     \
            return NOERROR;                                 \
        }                                                   \
        else if (object == (IInterface*)(InterfaceName3*)this) {    \
            iid = IID_##InterfaceName3;                     \
            return NOERROR;                                 \
        }                                                   \
        return SuperclassName::GetInterfaceID(object, iid); \
    }
#endif

#ifndef COMO_INTERFACE_REFCOUNT
#define COMO_INTERFACE_REFCOUNT(ClassName)                      \
    Integer ClassName::AddRef(                                  \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::AddRef(id);                              \
    }                                                           \
                                                                \
    Integer ClassName::Release(                                 \
        /* [in] */ HANDLE id)                                   \
    {                                                           \
        return Object::Release(id);                             \
    }
#endif

#ifndef COMO_INTERFACE_PROBE_BEGIN
#define COMO_INTERFACE_PROBE_BEGIN(ClassName)                   \
    IInterface* ClassName::Probe(                               \
        /* [in] */ const InterfaceID& iid)                      \
    {                                                           \
        if (iid == IID_IInterface) {                            \
            return (IInterface*)(IObject*)this;                 \
        }
#endif

#ifndef COMO_INTERFACE_PROBE_INTERFACE
#define COMO_INTERFACE_PROBE_INTERFACE(InterfaceName)           \
    else if (iid == IID_##InterfaceName) {                      \
        return (InterfaceName*)this;                            \
    }
#endif

#ifndef COMO_INTERFACE_PROBE_NESTEDINTERFACE
#define COMO_INTERFACE_PROBE_NESTEDINTERFACE(OuterInterfaceName, InterfaceName)  \
    else if (iid == OuterInterfaceName::IID_##InterfaceName) {  \
        return (OuterInterfaceName::InterfaceName*)this;        \
    }
#endif

#ifndef COMO_INTERFACE_PROBE_END
#define COMO_INTERFACE_PROBE_END(SuperclassName)                \
        return SuperclassName::Probe(iid);                      \
    }
#endif

#ifndef COMO_INTERFACE_GETINTERFACEID_BEGIN
#define COMO_INTERFACE_GETINTERFACEID_BEGIN(ClassName)          \
    ECode ClassName::GetInterfaceID(                            \
        /* [in] */ IInterface* object,                          \
        /* [out] */ InterfaceID& iid)                           \
    {                                                           \
        if (object == (IInterface*)(IObject*)this) {            \
            iid = IID_##IObject;                                \
            return NOERROR;                                     \
        }
#endif

#ifndef COMO_INTERFACE_GETINTERFACEID_INTERFACE
#define COMO_INTERFACE_GETINTERFACEID_INTERFACE(InterfaceName)  \
    else if (object == (IInterface*)(InterfaceName*)this) {     \
        iid = IID_##InterfaceName;                              \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_GETINTERFACEID_NESTEDINTERFACE
#define COMO_INTERFACE_GETINTERFACEID_NESTEDINTERFACE(OuterInterfaceName, InterfaceName) \
    else if (object == (IInterface*)(OuterInterfaceName::InterfaceName*)this) {          \
        iid = OuterInterfaceName::IID_##InterfaceName;          \
        return NOERROR;                                         \
    }
#endif

#ifndef COMO_INTERFACE_GETINTERFACEID_END
#define COMO_INTERFACE_GETINTERFACEID_END(SuperclassName)       \
        return SuperclassName::GetInterfaceID(object, iid);     \
    }
#endif

#ifndef COMO_OBJECT_DECL
#define COMO_OBJECT_DECL()                                 \
    ECode GetCoclassID(                                    \
        /* [out] */ CoclassID& cid) override;
#endif

#ifndef COMO_OBJECT_IMPL
#define COMO_OBJECT_IMPL(ClassName)                        \
    ECode ClassName::GetCoclassID(                         \
        /* [out] */ CoclassID& cid)                        \
    {                                                      \
        cid = CID_##ClassName;                             \
        return NOERROR;                                    \
    }
#endif

} // namespace como

#endif // __COMO_CCMDEF_H__
