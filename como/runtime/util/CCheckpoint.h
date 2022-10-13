//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#ifndef __ComoCCheckpoint_H__
#define __ComoCCheckpoint_H__

#include "comotypes.h"
#include "comoref.h"
#include "util/hashmap.h"

using namespace std;

namespace como {

class COM_PUBLIC CCheckpoint
    : public ICheckpoint
{
public:
    CCheckpoint(Integer ckpointId)
        : mCkpointId(ckpointId)
    {}

    ECode Execute() override;

    ECode ExecuteStr(
        /* [in] */ const String& strBuffer) override;

    ECode SetTransId(
        /* [in] */ Long transId) override;

    ECode GetTransId(
        /* [out] */ Long& transId) override;

    ECode SetParam(
        /* [in] */ const String& name,
        /* [in] */ IObject* object) override;

    ECode GetParam(
        /* [in] */ const String& name,
        /* [out] */ AutoPtr<IObject>& object) override;

    ECode ClearParams() override;

private:
    Integer mCkpointId;
    Long    mTransId;
    HashMap<String, IObject*> mParams;
};

} // namespace como

#endif // __ComoCCheckpoint_H__
