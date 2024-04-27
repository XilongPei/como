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

/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __COMO_REFBASE_H__
#define __COMO_REFBASE_H__

#include "comotypes.h"
#include "comointfs.h"
#include <atomic>

namespace como {

// compile with refcounting debugging enabled
#define DEBUG_REFS                      1
#define DEBUG_REFS_LightRefBase         1
// #define DEBUG_REFS_FATAL_SANITY_CHECKS  0
#define DEBUG_REFS_ENABLED_BY_DEFAULT   1
#define DEBUG_REFS_CALLSTACK_ENABLED    0
// log all reference counting operations
#define PRINT_REFS                      1

using FREE_MEM_FUNCTION = void(*)(Short,const void*);

class COM_PUBLIC RefBase
{
public:
    class WeakRef
    {
    public:
        RefBase* GetRefBase() const;

        void IncWeak(
            /* [in] */ const void* id);

        void DecWeak(
            /* [in] */ const void* id);

        Boolean AttemptIncStrong(
            /* [in] */ const void* id);

        Boolean AttemptIncWeak(
            /* [in] */ const void* id);

        Integer GetWeakCount() const;

        void PrintRefs(const char* objInfo) const;

        void TrackMe(
            /* [in] */ Boolean enable,
            /* [in] */ Boolean retain);
    };

public:
    Integer IncStrong(
        /* [in] */ const void* id) const;

    Integer DecStrong(
        /* [in] */ const void* id) const;

    Integer ForceIncStrong(
        /* [in] */ const void* id) const;

    Integer GetStrongCount() const;

    WeakRef* CreateWeak(
        /* [in] */ const void* id) const;

    WeakRef* GetWeakRefs() const;

    inline void SetFunFreeMem(FREE_MEM_FUNCTION func, Short shortPara);

    inline void PrintRefs(const char* objInfo) const;

    inline void TrackMe(
        /* [in] */ Boolean enable,
        /* [in] */ Boolean retain);

    inline Integer AddRef(
        /* [in] */ HANDLE id = 0);

    inline Integer Release(
        /* [in] */ HANDLE id = 0);

protected:
    RefBase();

    virtual ~RefBase();

    // Flags for ExtendObjectLifetime()
    enum {
        OBJECT_LIFETIME_STRONG  = 0x0000,
        OBJECT_LIFETIME_WEAK    = 0x0001,
        OBJECT_LIFETIME_MASK    = 0x0001
    };

    void ExtendObjectLifetime(
        /* [in] */ Integer mode);

    virtual void OnFirstRef(
        /* [in] */ const void* id);

    virtual void OnLastStrongRef(
        /* [in] */ const void* id);

    // Flags for OnIncStrongAttempted()
    enum {
        FIRST_INC_STRONG = 0x0001
    };

    virtual Boolean OnIncStrongAttempted(
        /* [in] */ Integer flags,
        /* [in] */ const void* id);

    virtual void OnLastWeakRef(
        /* [in] */ const void* id);

private:
    friend class WeakRef;
    class WeakRefImpl;

    RefBase(
        /* [in] */ const RefBase& o);

    RefBase& operator=(
        /* [in] */ const RefBase& o);

    WeakRefImpl* const mRefs;
    HANDLE funFreeMem;
};

void RefBase::PrintRefs(const char* objInfo) const
{
    GetWeakRefs()->PrintRefs(objInfo);
}

void RefBase::TrackMe(
    /* [in] */ Boolean enable,
    /* [in] */ Boolean retain)
{
    GetWeakRefs()->TrackMe(enable, retain);
}

Integer RefBase::AddRef(
    /* [in] */ HANDLE id)
{
    return IncStrong(reinterpret_cast<const void*>(id));
}

Integer RefBase::Release(
    /* [in] */ HANDLE id)
{
    return DecStrong(reinterpret_cast<const void*>(id));
}

void RefBase::SetFunFreeMem(FREE_MEM_FUNCTION func, Short shortPara)
{
    funFreeMem = (static_cast<HANDLE>(shortPara) << 48) |
                              (reinterpret_cast<HANDLE>(func) & 0xFFFFFFFFFFFF);
                                                                // 5 4 3 2 1 0
}

/**
 * LightRefBase, base class of COMO object.
 */
class COM_PUBLIC LightRefBase
{
public:
    LightRefBase();

    Integer AddRef(
        /* [in] */ HANDLE id = 0);

    Integer Release(
        /* [in] */ HANDLE id = 0);

    IInterface* Probe(
        /* [in] */ const InterfaceID& iid) const;

    ECode GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID& iid) const;

    Integer GetStrongCount() const;

    void SetFunFreeMem(FREE_MEM_FUNCTION func, Short shortPara);

protected:
    virtual ~LightRefBase();

private:
    mutable std::atomic<Integer> mCount;
    HANDLE funFreeMem;

#if DEBUG_REFS_LightRefBase
public:
    struct RefEntry
    {
        RefEntry* mNext;
        HANDLE mId;
  #if DEBUG_REFS_CALLSTACK_ENABLED
        CallStack mStack;
  #endif
        Integer mRef;
    };
    Boolean mTrackEnabled;
    Boolean mRetain;
    mutable Mutex mMutex;
    RefEntry* mRefs;

    void PrintRefs(const char* objInfo);

    void TrackMe(
        /* [in] */ Boolean track,
        /* [in] */ Boolean retain);
#endif
};

// WeakReferenceImpl
class COM_PUBLIC WeakReferenceImpl
    : public LightRefBase
    , public IWeakReference
{
public:
    WeakReferenceImpl(
        /* [in] */ IInterface* object,
        /* [in] */ RefBase::WeakRef* ref);

    ~WeakReferenceImpl();

    COMO_INTERFACE_DECL();

    ECode Resolve(
        /* [in] */ const InterfaceID& iid,
        /* [out] */ IInterface** object) override;

private:
    IInterface* mObject;
    RefBase::WeakRef* mRef;
};

} // namespace como

#endif // __COMO_REFBASE_H__
