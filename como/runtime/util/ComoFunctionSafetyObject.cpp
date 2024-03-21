//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

//
// when building environment has $ENV{COMO_FUNCTION_SAFETY}, it will define
// -DCOMO_FUNCTION_SAFETY=$ENV{COMO_FUNCTION_SAFETY}

#ifdef COMO_FUNCTION_SAFETY
#endif

#include <time.h>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include "comoobj.h"
#include "comoapi.h"
#include "comolog.h"
#include "mistring.h"
#include "ini.h"
#include "ComoFunctionSafetyObject.h"

using namespace std;

namespace como {

static pthread_mutex_t funSafetyLock;

class SoelfComoFunctionSafetyObject
{
public:
    SoelfComoFunctionSafetyObject();
    ~SoelfComoFunctionSafetyObject();
};
static SoelfComoFunctionSafetyObject soelfComoFunctionSafetyObject;

// define an instance of CFSO_VECTOR, Class Function Safety Object management VECTOR
// All the objects for overdue management are in this variable
static CFSO_VECTOR objsLifeCycleExpires;

//
// class SoelfComoFunctionSafetyObject
//
SoelfComoFunctionSafetyObject::SoelfComoFunctionSafetyObject()
{
    pthread_mutex_init(&funSafetyLock, nullptr);
}

SoelfComoFunctionSafetyObject::~SoelfComoFunctionSafetyObject()
{
    pthread_mutex_destroy(&funSafetyLock);
}

//
// class ComoFunctionSafetyObject
//
ComoFunctionSafetyObject::ComoFunctionSafetyObject()
    : mIsValid(0)
    , mChecksum(0)
    , mExpires(0)
{}

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (/*strcmp(section, "") == 0 && */strcmp(name, "expires") == 0) {
        (*(Long*)user) = atol(value);
    }
    return 1;
}

ECode ComoFunctionSafetyObject::AfterConstruction()
{
    AutoPtr<IMetaCoclass> klass;
    IObject::Probe(this)->GetCoclass(klass);
    String funcSafetySetting;
    klass->GetFuncSafetySetting(funcSafetySetting);

    if (funcSafetySetting.IsEmpty()) {
        mExpires = CFSO_ExpireVALID;
    }
    else {
        // parse funcSafetySetting
        if (ini_parse_string(funcSafetySetting.string(), handler, &mExpires) < 0) {
            mExpires = CFSO_ExpireVALID;
        }
    }

    clock_gettime(CLOCK_REALTIME, &mLastModifiedTime);
    pthread_mutex_lock(&funSafetyLock);
    if (objsLifeCycleExpires.cfso_push(this) < 0) {
        Logger::E("ComoFunctionSafetyObject", "Construct Object error");
    }
    pthread_mutex_unlock(&funSafetyLock);

    return NOERROR;
}

ComoFunctionSafetyObject::~ComoFunctionSafetyObject()
{
    if (mExpires > 0) {
        pthread_mutex_lock(&funSafetyLock);
        int index = objsLifeCycleExpires.cfso_find(this);
        if (index >= 0) {
            objsLifeCycleExpires.cfso_del(index);
        }
        else {
            Logger::E("ComoFunctionSafetyObject", "lost object: %lx", this);
        }
        pthread_mutex_unlock(&funSafetyLock);
    }
}

Long ComoFunctionSafetyObject::GetExpires()
{
    return mExpires;
}

ECode ComoFunctionSafetyObject::SetExpires(
    /* [out] */ Long expires)
{
    mExpires = expires;

    if (objsLifeCycleExpires.cfso_find(this) < 0) {
        objsLifeCycleExpires.cfso_push(this);
    }

    return NOERROR;
}

ECode ComoFunctionSafetyObject::SetLastModifiedInfo()
{
    clock_gettime(CLOCK_REALTIME, &mLastModifiedTime);

    return NOERROR;
}

/**
 * default function for checking ComoFunctionSafetyObject
 */
ECode ComoFunctionSafetyObject::IsValid(
        /* [out] */ Integer& isValid)
{
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);

    if (1000000000L * (mLastModifiedTime.tv_sec - time.tv_sec) +
       /*987654321*/    (mLastModifiedTime.tv_nsec - time.tv_nsec) > mExpires) {

        isValid = CFSO_ExpireTime;
        return NOERROR;
    }

    isValid = mIsValid;
    return NOERROR;
}

/**
 * default function for setting property isValid of ComoFunctionSafetyObject
 * if parameter isValid is 0, means set Object valid
 */
ECode ComoFunctionSafetyObject::InvalidObject(
        /* [out] */ Integer isValid)
{
    mIsValid = isValid;
    return NOERROR;
}

/**
 * Take out the last checksum and the current Object's checksum
 */
ECode ComoFunctionSafetyObject::GetChecksum(
        /* [out] */ Long& lastChecksum,
        /* [out] */ Long& currentChecksum)
{
    lastChecksum = mChecksum;

    // During CRC calculation, mCheckSum should be temporarily cleared to 0,
    // because mCheckSum is also a part of the object, and CRC calculation result
    // will be stored in mCheckSum, resulting in inconsistency between the object
    // and before CRC calculation
    mChecksum = 0L;

    currentChecksum = mChecksum = Object::GetCRC64((IInterface*)this);
    return NOERROR;
}

//
// class CFSO_VECTOR, Class Function Safety Object management VECTOR
//-------------------------------------------------------------------------

CFSO_VECTOR::CFSO_VECTOR()
{
    _size = 0;
    _extra = 0;
    extra = CFSO_VECTOR_Size_ONCE_ALLOC;
    numNullArray = 0;
    cfso_allocate();
}

CFSO_VECTOR::~CFSO_VECTOR()
{
    free(_data);
}

unsigned int CFSO_VECTOR::cfso_allocate()
{
    ComoFunctionSafetyObject *newData = (ComoFunctionSafetyObject *)realloc(_data,
                                        sizeof(ComoFunctionSafetyObject*) * (_size + extra));

    if (nullptr == newData) {
        _extra = 0;
        _data = nullptr;
        Logger::D("CFSO_VECTOR::cfso_allocate", "realloc error");
    }
    else {
        _extra = extra;
        _data = newData;
    }
    return _extra;
}

int CFSO_VECTOR::cfso_push(ComoFunctionSafetyObject *cfso) {
    if (numNullArray > 0) {
        int index = idxNullArray[numNullArray--];
        *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) + index *
                                                    sizeof(ComoFunctionSafetyObject*)) = cfso;
        return index;
    }

    if (_extra < 1) {
#ifdef COMO_FUNCTION_SAFETY_RTOS
        return -1;
#else
        if (cfso_allocate() <= 0)
            return -1;
#endif
    }

    *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) + _size *
                                                    sizeof(ComoFunctionSafetyObject*)) = cfso;
    _size++;
    _extra--;

    return 0;
}

ComoFunctionSafetyObject *CFSO_VECTOR::cfso_get(int index) {
    if ((_size < 1) || (index >= _size)) {
        return nullptr;
    }
    return *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) +
                                                        index * sizeof(ComoFunctionSafetyObject*));
}

int CFSO_VECTOR::cfso_find(ComoFunctionSafetyObject *cfso)
{
    for (int i = 0;  i < _size;  i++) {
        if (cfso == *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) + i *
                                                            sizeof(ComoFunctionSafetyObject*))) {
            return i;
        }
    }

    return -1;
}

int CFSO_VECTOR::cfso_del(int index) {
    if ((_size < 1) || (index >= _size)) {
        return -1;
    }

    if (numNullArray >= CFSO_VECTOR_SizeNullArray) {
        for ( ;  numNullArray >= 0;  numNullArray--) {
            ComoFunctionSafetyObject* cfso;
            for ( ;  _size > 0;  _size--) {
                cfso = *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) +
                                                        (_size-1) * sizeof(ComoFunctionSafetyObject*));
                if (cfso != nullptr)
                    break;
            }
            if (cfso != nullptr) {
                *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) +
                            idxNullArray[numNullArray-1] * sizeof(ComoFunctionSafetyObject*)) = cfso;
            }
            else {
                numNullArray = 0;
                break;
            }
        }
    }

    idxNullArray[numNullArray++] = index;
    *(ComoFunctionSafetyObject**)(reinterpret_cast<HANDLE>(_data) +
                                                index * sizeof(ComoFunctionSafetyObject*)) = nullptr;
    return 0;
}

} // namespace como
