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

#include <stdio.h>
#include "FSOGetterSetter.h"

//namespace como {

extern "C" void FSOGetterChecker(char *s)
{
    printf("FSOGetterChecker %s\n", s);
}

extern "C" void FSOSetterChecker(char *s)
{
    printf("FSOSetterChecker %s\n", s);
}

extern "C" void FSOGetter(PropertyName, char *s)
{
    puts(s);
}

extern "C" void FSOSetter(PropertyName, char *s)
{
    puts(s);
}

//} // namespace como

int main()
{
    CallFSOGetter(PropertyName, (char*)"===test main CallFSOGetter====\n");
    CallFSOSetter(PropertyName, (char*)"===test main CallFSOSetter====\n");
}
