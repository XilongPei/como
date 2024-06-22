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

#include "ast/Constant.h"
#include "ast/Module.h"
#include "ast/Namespace.h"
#include "util/StringBuilder.h"

namespace cdlc {

String Constant::ToString()
{
    return mNamespace->ToString() + "::" + mName;
}

String Constant::Dump(
    /* [in] */ const String& prefix)
{
    StringBuilder builder;

    builder.Append(prefix).Append("Constant[");
    builder.AppendFormat("name:%s, ", mName.string());
    builder.AppendFormat("namespace:%s, ", mNamespace->ToString().string());
    builder.AppendFormat("type:%s, ", mType->ToString().string());

    if (mType->IsBooleanType()) {
        bool b = mValue->BooleanValue();
        builder.AppendFormat("value:%d", b);
    }
    else if (mType->IsIntegralType()) {
        long l = mValue->LongValue();
        builder.AppendFormat("value:%ld", l);
    }
    else if (mType->IsFloatingPointType()) {
         double f = mValue->DoubleValue();
         builder.AppendFormat("value:%lf", f);
    }
    else if (mType->IsStringType()) {
        String str = mValue->StringValue();
        builder.AppendFormat("value:%s", str.string());
    }
    else if (mType->IsEnumerationType()) {
        String str = mValue->EnumeratorValue();
        builder.AppendFormat("value:%s", str.string());
    }
    else {
        builder.AppendFormat("value:%s", mValue->ToString().string());
    }

    builder.Append("]\n");
    return builder.ToString();
}

AutoPtr<Node> Constant::Clone(
    /* [in] */ Module *module,
    /* [in] */ bool deepCopy)
{
    AutoPtr<Constant> clone = new Constant();
    if (nullptr == clone) {
        return nullptr;
    }
    clone->mName = mName;
    if (! deepCopy) {
        clone->mType = mType;
    }
    else {
        AutoPtr<Type> type = module->FindType(mType->ToString());
        if (nullptr == type) {
            type = mType->Clone(module, false);;
        }
        clone->mType = type;
    }
    clone->mValue = mValue;
    clone->mNamespace = mNamespace;
    return clone;
}

} // namespace cdlc
