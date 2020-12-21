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

#include "CFooBar.h"
#include <cstdio>

namespace como {
namespace demo {

COMO_INTERFACE_IMPL_2(CFooBar, Object, IFoo, IBar);

CFooBar::CFooBar()
{
    printf("==== Call CFooBar() ====\n");
}

CFooBar::~CFooBar()
{
    printf("==== Call ~CFooBar() ====\n");
}

ECode CFooBar::Constructor()
{
    return NOERROR;
}

ECode CFooBar::Constructor(
    /* [in] */ Long data)
{
    return NOERROR;
}

ECode CFooBar::Foo(
    /* [in] */ Integer data)
{
    return NOERROR;
}

ECode CFooBar::Bar(
    /* [in] */ const String& data)
{
    return NOERROR;
}

}
}
