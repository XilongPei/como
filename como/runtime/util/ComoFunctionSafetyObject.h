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

#ifndef __ComoFunctionSafetyObject_H__
#define __ComoFunctionSafetyObject_H__

#include <time.h>
#include <vector>
#include "comotypes.h"
#include "comoref.h"

using namespace std;

namespace como {

class COM_PUBLIC ComoFunctionSafetyObject
    : public IComoFunctionSafetyObject
{
public:
    ComoFunctionSafetyObject();
    ~ComoFunctionSafetyObject();
    ECode AfterConstruction();

    Long GetExpires();

    ECode SetExpires(
        /* [in] */ Long expires);

    ECode SetLastModifiedInfo();

    /**
     * default function for checking ComoFunctionSafetyObject
     */
    ECode IsValid(
            /* [out] */ Integer& isValid) override;

    /**
     * default function for setting property isValid of ComoFunctionSafetyObject
     * if parameter isValid is 0, means set Object valid
     */
    ECode InvalidObject(
            /* [out] */ Integer isValid) override;

    /**
     * Take out the last checksum and the current Object's checksum
     */
    ECode GetChecksum(
            /* [out] */ Long& lastChecksum,
            /* [out] */ Long& currentChecksum) override;
private:
    Integer mIsValid;                   // value definition: 0:valid; others:invalid
    Long mChecksum;
    Long mExpires;
    struct timespec mLastModifiedTime;  // CLOCK_REALTIME
};

enum CFSO_Expires {
    CFSO_ExpireVALID = 0,
    CFSO_ExpireTime
};

#define CFSO_VECTOR_Size_ONCE_ALLOC 1024
#define CFSO_VECTOR_SizeNullArray   300

// Class Function Safety Object management VECTOR
class CFSO_VECTOR
{
public:
    CFSO_VECTOR();
    ~CFSO_VECTOR();

    int cfso_push(ComoFunctionSafetyObject *cfso);
    int cfso_del(unsigned int index);
    ComoFunctionSafetyObject *cfso_get(int index);
    int cfso_find(ComoFunctionSafetyObject *cfso);

private:
    unsigned int cfso_allocate();

    ComoFunctionSafetyObject *_data;    // the data
    unsigned int _size;     // number of ComoFunctionSafetyObject *
    unsigned int _extra;    // how much extra space is left
    unsigned int extra;     // just how much extra space we should add

    int idxNullArray[CFSO_VECTOR_SizeNullArray];
    int numNullArray;
};

} // namespace como

#endif // __ComoFunctionSafetyObject_H__
