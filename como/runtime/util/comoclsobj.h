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

#ifndef __COMO_CLASSOBJECT_H__
#define __COMO_CLASSOBJECT_H__

#include "comoobj.h"

namespace como {

class COM_PUBLIC ClassObject
    : public Object
    , public IClassObject
{
public:
    COMO_INTERFACE_DECL();

    ECode AttachMetadata(
        /* [in] */ IMetaComponent* component) override;

    ECode GetMetadate(
        /* [out] */ AutoPtr<IMetaComponent>& component) override;

protected:
    IMetaComponent* mComponent = nullptr;
};

} // namespace como

#endif //__COMO_CLASSOBJECT_H__
