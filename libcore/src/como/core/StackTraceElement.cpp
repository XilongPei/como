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

#include "como/core/CStringBuilder.h"
#include "como/core/StackTraceElement.h"
#include <comosp.h>

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_2(StackTraceElement, SyncObject, IStackTraceElement, ISerializable);

ECode StackTraceElement::Constructor(
    /* [in] */ const String& no,
    /* [in] */ const String& pc,
    /* [in] */ const String& soname,
    /* [in] */ const String& symbol)
{
    mNo = no;
    mPC = pc;
    mSoname = soname;
    mSymbol = symbol;
    return NOERROR;
}

ECode StackTraceElement::ToString(
    /* [out] */ String& desc)
{
    AutoPtr<IStringBuilder> sb;
    CStringBuilder::New(IID_IStringBuilder, (IInterface**)&sb);
    sb->Append(mNo);
    sb->Append(String("  "));
    sb->Append(mPC);
    sb->Append(U' ');
    sb->Append(mSoname);
    if (!mSymbol.IsEmpty()) {
        sb->Append(U' ');
        sb->Append(mSymbol);
    }
    return sb->ToString(desc);
}

}
}
