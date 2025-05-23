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

#ifndef __COMO_AUTOPTR_H__
#define __COMO_AUTOPTR_H__

#include "comotypes.h"
#include <utility>
#ifdef COMO_FUNCTION_SAFETY
#include "VerifiedU64Pointer.h"
#endif

namespace como {

template<typename T>
class AutoPtr
{
public:
    inline AutoPtr()
        : mPtr(nullptr)
    {}

    AutoPtr(
        /* [in] */ T* other);

    AutoPtr(
        /* [in] */ const AutoPtr<T>& other);

    AutoPtr(
        /* [in] */ AutoPtr<T>&& other);

    template<typename U>
    AutoPtr(
        /* [in] */ U* other);

    template<typename U>
    AutoPtr(
        /* [in] */ const AutoPtr<U>& other);

    template<typename U>
    AutoPtr(
        /* [in] */ AutoPtr<U>&& other);

    ~AutoPtr();

    AutoPtr& operator=(
        /* [in] */ T* other);

    AutoPtr& operator=(
        /* [in] */ const AutoPtr<T>& other);

    AutoPtr& operator=(
        /* [in] */ AutoPtr<T>&& other);

    template<typename U>
    AutoPtr& operator=(
        /* [in] */ U* other);

    template<typename U>
    AutoPtr& operator=(
        /* [in] */ const AutoPtr<U>& other);

    template<typename U>
    AutoPtr& operator=(
        /* [in] */ AutoPtr<U>&& other);

    void MoveTo(
        /* [out] */ T** other);

    template<typename U>
    void MoveTo(
        /* [out] */ U** other);

    inline operator T*() const;

    inline operator AutoPtr<IInterface>&() const;

    inline T** operator&();

    inline T* operator->() const;

    inline T& operator*() const;

    inline T* Get() const;

    inline Boolean operator==(
        /* [in] */ T* other) const;

    inline Boolean operator==(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator==(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator==(
        /* [in] */ const AutoPtr<U>& other) const;

    inline Boolean operator!=(
        /* [in] */ T* other) const;

    inline Boolean operator!=(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator!=(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator!=(
        /* [in] */ const AutoPtr<U>& other) const;

    inline Boolean operator>(
        /* [in] */ T* other) const;

    inline Boolean operator>(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator>(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator>(
        /* [in] */ const AutoPtr<U>& other) const;

    inline Boolean operator<(
        /* [in] */ T* other) const;

    inline Boolean operator<(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator<(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator<(
        /* [in] */ const AutoPtr<U>& other) const;

    inline Boolean operator>=(
        /* [in] */ T* other) const;

    inline Boolean operator>=(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator>=(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator>=(
        /* [in] */ const AutoPtr<U>& other) const;

    inline Boolean operator<=(
        /* [in] */ T* other) const;

    inline Boolean operator<=(
        /* [in] */ const AutoPtr<T>& other) const;

    template<typename U>
    inline Boolean operator<=(
        /* [in] */ U* other) const;

    template<typename U>
    inline Boolean operator<=(
        /* [in] */ const AutoPtr<U>& other) const;

private:
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct CopyConstructorImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct MoveAssignImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct MoveToImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct EqualImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct NotEqualImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct GreaterImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct LessImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct GreaterEqualImpl;
    template<typename U, typename Y, Boolean isSuperSubclass> friend struct LessEqualImpl;
    template<typename U, Boolean isIInterfaceSubclass> friend struct AutoPtrConvertImpl;

    T* mPtr;
};

template<typename U, typename Y, Boolean isSuperSubclass>
struct CopyConstructorImpl
{
    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            uObj->AddRef(reinterpret_cast<HANDLE>(this));
        }
    }

    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.mPtr);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            uObj->AddRef(reinterpret_cast<HANDLE>(this));
        }
    }

    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ AutoPtr<Y>&& rvalue)
    {
        U* uObj = U::Probe(rvalue.mPtr);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            rvalue.mPtr = nullptr;
        }
    }
};

template<typename U, typename Y>
struct CopyConstructorImpl<U, Y, true>
{
    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        lvalue->mPtr = (U*)rvalue;
        rvalue->AddRef(reinterpret_cast<HANDLE>(this));
    }

    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        lvalue->mPtr = (U*)rvalue.mPtr;
        rvalue->AddRef(reinterpret_cast<HANDLE>(this));
    }

    void operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ AutoPtr<Y>&& rvalue)
    {
        lvalue->mPtr = (U*)rvalue.mPtr;
        rvalue.mPtr = nullptr;
    }
};

template<typename U, typename Y, Boolean isSuperSubclass>
struct MoveAssignImpl
{
    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        U* uObj = U::Probe(rvalue);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            uObj->AddRef(reinterpret_cast<HANDLE>(this));
        }
        return *lvalue;
    }

    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        U* uObj = U::Probe(rvalue.mPtr);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            uObj->AddRef(reinterpret_cast<HANDLE>(this));
        }
        return *lvalue;
    }

    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ AutoPtr<Y>&& rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        U* uObj = U::Probe(rvalue.mPtr);
        lvalue->mPtr = uObj;
        if (uObj != nullptr) {
            rvalue.mPtr = nullptr;
        }
        return *lvalue;
    }
};

template<typename U, typename Y>
struct MoveAssignImpl<U, Y, true>
{
    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        lvalue->mPtr = (U*)rvalue;
        rvalue->AddRef(reinterpret_cast<HANDLE>(this));
        return *lvalue;
    }

    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        lvalue->mPtr = (U*)rvalue.mPtr;
        rvalue->AddRef(reinterpret_cast<HANDLE>(this));
        return *lvalue;
    }

    AutoPtr<U>& operator()(
        /* [in] */ AutoPtr<U>* lvalue,
        /* [in] */ AutoPtr<Y>&& rvalue)
    {
        if (lvalue->mPtr != nullptr) {
            lvalue->mPtr->Release(reinterpret_cast<HANDLE>(this));
        }
        lvalue->mPtr = (U*)rvalue.mPtr;
        rvalue.mPtr = nullptr;
        return *lvalue;
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct MoveToImpl
{
    void operator()(
        /* [in] */ AutoPtr<U>* uObj,
        /* [in] */ Y** other)
    {
        if (other != nullptr) {
            Y* yObj = Y::Probe(uObj->mPtr);
            if (yObj != nullptr) {
                *other = yObj;
                uObj->mPtr = nullptr;
            }
        }
    }
};

template<typename U, typename Y>
struct MoveToImpl<U, Y, true>
{
    void operator()(
        /* [in] */ AutoPtr<U>* uObj,
        /* [in] */ Y** other)
    {
        if (other != nullptr) {
            *other = (Y*)uObj->mPtr;
            uObj->mPtr = nullptr;
        }
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct EqualImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr == uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr == uObj;
    }
};

template<typename U, typename Y>
struct EqualImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr == (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr == (U*)rvalue.Get();
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct NotEqualImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr != uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr != uObj;
    }
};

template<typename U, typename Y>
struct NotEqualImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr != (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr != (U*)rvalue.Get();
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct GreaterImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr > uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr > uObj;
    }
};

template<typename U, typename Y>
struct GreaterImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr > (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr > (U*)rvalue.Get();
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct LessImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr < uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr < uObj;
    }
};

template<typename U, typename Y>
struct LessImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr < (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr < (U*)rvalue.Get();
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct GreaterEqualImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr >= uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr >= uObj;
    }
};

template<typename U, typename Y>
struct GreaterEqualImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr >= (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr >= (U*)rvalue.Get();
    }
};

template<typename U, typename Y, Boolean isSubSuperclass>
struct LessEqualImpl
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        U* uObj = U::Probe(rvalue);
        return lvalue->mPtr <= uObj;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        U* uObj = U::Probe(rvalue.Get());
        return lvalue->mPtr <= uObj;
    }
};

template<typename U, typename Y>
struct LessEqualImpl<U, Y, true>
{
    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ Y* rvalue)
    {
        return lvalue->mPtr <= (U*)rvalue;
    }

    Boolean operator()(
        /* [in] */ const AutoPtr<U>* lvalue,
        /* [in] */ const AutoPtr<Y>& rvalue)
    {
        return lvalue->mPtr <= (U*)rvalue.Get();
    }
};

template<typename U, Boolean isIInterfaceSubclass>
struct AutoPtrConvertImpl
{
    AutoPtr<IInterface>& operator()(
        /* [in] */ const AutoPtr<U>* lvalue)
    {
        return *static_cast<AutoPtr<IInterface>*>(nullptr);
    }
};

template<typename U>
struct AutoPtrConvertImpl<U, true>
{
    AutoPtr<IInterface>& operator()(
        /* [in] */ const AutoPtr<U>* lvalue)
    {
        return *reinterpret_cast<AutoPtr<IInterface>*>(const_cast<AutoPtr<U>*>(lvalue));
    }
};

template<typename T>
AutoPtr<T>::AutoPtr(
    /* [in] */ T* other)
    : mPtr(other)
{
    if (mPtr != nullptr) {
        mPtr->AddRef(reinterpret_cast<HANDLE>(this));
    }
}

template<typename T>
AutoPtr<T>::AutoPtr(
    /* [in] */ const AutoPtr<T>& other)
    : mPtr(other.mPtr)
{
    if (mPtr != nullptr) {
        mPtr->AddRef(reinterpret_cast<HANDLE>(this));
    }
}

template<typename T>
AutoPtr<T>::AutoPtr(
    /* [in] */ AutoPtr<T>&& other)
    : mPtr(other.mPtr)
{
    other.mPtr = nullptr;
}

template<typename T> template<typename U>
AutoPtr<T>::AutoPtr(
    /* [in] */ U* other)
{
    CopyConstructorImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    impl(this, other);
}

template<typename T> template<typename U>
AutoPtr<T>::AutoPtr(
    /* [in] */ const AutoPtr<U>& other)
{
    CopyConstructorImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    impl(this, other);
}

template<typename T> template<typename U>
AutoPtr<T>::AutoPtr(
    /* [in] */ AutoPtr<U>&& other)
{
    CopyConstructorImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    impl(this, std::move(other));
}

template<typename T>
AutoPtr<T>::~AutoPtr()
{
    if (mPtr != nullptr) {
        mPtr->Release(reinterpret_cast<HANDLE>(this));
    }
}

template<typename T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ T* other)
{
    if (mPtr == other) {
        return *this;
    }

    if (other != nullptr) {
        other->AddRef(reinterpret_cast<HANDLE>(this));
    }
    if (mPtr != nullptr) {
        mPtr->Release(reinterpret_cast<HANDLE>(this));
    }
    mPtr = other;
    return *this;
}

template<typename T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ const AutoPtr<T>& other)
{
    if (mPtr == other.mPtr) {
        return *this;
    }

    if (other.mPtr != nullptr) {
        other.mPtr->AddRef(reinterpret_cast<HANDLE>(this));
    }
    if (mPtr != nullptr) {
        mPtr->Release(reinterpret_cast<HANDLE>(this));
    }
    mPtr = other.mPtr;
    return *this;
}

template<typename T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ AutoPtr<T>&& other)
{
    if (mPtr != nullptr) {
        mPtr->Release(reinterpret_cast<HANDLE>(this));
    }
    mPtr = other.mPtr;
    other.mPtr = nullptr;
    return *this;
}

template<typename T> template<typename U>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ U* other)
{
    MoveAssignImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ const AutoPtr<U>& other)
{
    MoveAssignImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ AutoPtr<U>&& other)
{
    MoveAssignImpl<T, U, SUPERSUBCLASS(T, U)> impl;
    return impl(this, std::move(other));
}

template<typename T>
void AutoPtr<T>::MoveTo(
    /* [out] */ T** other)
{
    if (other != nullptr) {
        *other = mPtr;
        mPtr = nullptr;
    }
}

template<typename T> template<typename U>
void AutoPtr<T>::MoveTo(
    /* [out] */ U** other)
{
    MoveToImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
AutoPtr<T>::operator T*() const
{
    return mPtr;
}

template<typename T>
AutoPtr<T>::operator AutoPtr<IInterface>&() const
{
    AutoPtrConvertImpl<T, SUPERSUBCLASS(IInterface, T)> impl;
    return impl(this);
}

template<typename T>
T** AutoPtr<T>::operator&()
{
    return &mPtr;
}

template<typename T>
T* AutoPtr<T>::operator->() const
{
    return mPtr;
}

template<typename T>
T& AutoPtr<T>::operator*() const
{
    return *mPtr;
}

template<typename T>
T* AutoPtr<T>::Get() const
{
    return mPtr;
}

template<typename T>
Boolean AutoPtr<T>::operator==(
    /* [in] */ T* other) const
{
    return mPtr == other;
}

template<typename T>
Boolean AutoPtr<T>::operator==(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr == other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator==(
    /* [in] */ U* other) const
{
    EqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator==(
    /* [in] */ const AutoPtr<U>& other) const
{
    EqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
Boolean AutoPtr<T>::operator!=(
    /* [in] */ T* other) const
{
    return mPtr != other;
}

template<typename T>
Boolean AutoPtr<T>::operator!=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr != other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator!=(
    /* [in] */ U* other) const
{
    NotEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator!=(
    /* [in] */ const AutoPtr<U>& other) const
{
    NotEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
Boolean AutoPtr<T>::operator>(
    /* [in] */ T* other) const
{
    return mPtr > other;
}

template<typename T>
Boolean AutoPtr<T>::operator>(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr > other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator>(
    /* [in] */ U* other) const
{
    GreaterImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator>(
    /* [in] */ const AutoPtr<U>& other) const
{
    GreaterImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
Boolean AutoPtr<T>::operator<(
    /* [in] */ T* other) const
{
    return mPtr < other;
}

template<typename T>
Boolean AutoPtr<T>::operator<(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr < other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator<(
    /* [in] */ U* other) const
{
    LessImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator<(
    /* [in] */ const AutoPtr<U>& other) const
{
    LessImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
Boolean AutoPtr<T>::operator>=(
    /* [in] */ T* other) const
{
    return mPtr >= other;
}

template<typename T>
Boolean AutoPtr<T>::operator>=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr >= other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator>=(
    /* [in] */ U* other) const
{
    GreaterEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator>=(
    /* [in] */ const AutoPtr<U>& other) const
{
    GreaterEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T>
Boolean AutoPtr<T>::operator<=(
    /* [in] */ T* other) const
{
    return mPtr <= other;
}

template<typename T>
Boolean AutoPtr<T>::operator<=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr <= other.mPtr;
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator<=(
    /* [in] */ U* other) const
{
    LessEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

template<typename T> template<typename U>
Boolean AutoPtr<T>::operator<=(
    /* [in] */ const AutoPtr<U>& other) const
{
    LessEqualImpl<T, U, SUPERSUBCLASS(U, T)> impl;
    return impl(this, other);
}

} // namespace como

#endif //__COMO_AUTOPTR_H__
