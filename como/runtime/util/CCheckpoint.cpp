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

#include "comoobj.h"
#include "comoapi.h"
#include "comolog.h"
#include "RuntimeMonitor.h"
#include "CCheckpoint.h"

namespace como {

static void CheckpointParamsWalker(String& str, String& name, IObject*& intf)
{
    String info;
    intf->ToString(info);
    str += "{\"" + name + "\":\"" + info + "\"},";
}

ECode CCheckpoint::Execute()
{
    String strBuffer;

    strBuffer = "[";
    mParams.GetValues(strBuffer, CheckpointParamsWalker);

    if (strBuffer.GetByteLength() > 1) {
        // replace the last ',' with ']'
        char* str = (char*)strBuffer.string();
        str[strBuffer.GetByteLength() - 1] = ']';
    }
    else {
        strBuffer += "]";
    }

    // tell RuntimeMonitor
    RuntimeMonitor::WriteLog(strBuffer.string(), strBuffer.GetByteLength());

    return NOERROR;
}

ECode CCheckpoint::SetParam(
    /* [in] */ String& name,
    /* [in] */ IObject* object)
{
    mParams.Put(name, object);
    return NOERROR;
}

ECode CCheckpoint::GetParam(
    /* [in] */ String& name,
    /* [out] */ AutoPtr<IObject>& object)
{
    object = mParams.Get(name);
    if (nullptr == object)
        return E_NOT_FOUND_EXCEPTION;

    return NOERROR;
}

} // namespace como
