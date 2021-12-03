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

#ifndef __FSOGetterSetter_h__
#define __FSOGetterSetter_h__

namespace como {

#define _FSO_MAKE_FUNC_GET_NAME_inside_(n) _get_##n
#define FSO_MAKE_FUNC_GET_NAME_inside(n) _FSO_MAKE_FUNC_GET_NAME_inside_(n)

#define _FSO_MAKE_FUNC_GET_NAME_(n) get##n
#define FSO_MAKE_FUNC_GET_NAME(n) _FSO_MAKE_FUNC_GET_NAME_(n)
#define callFSOGetter(fieldName,args...) FSO_MAKE_FUNC_GET_NAME(fieldName)(args)

#define _FSO_MAKE_FUNC_SET_NAME_inside_(n) _set_##n
#define FSO_MAKE_FUNC_SET_NAME_inside(n) _FSO_MAKE_FUNC_SET_NAME_inside_(n)

#define _FSO_MAKE_FUNC_SET_NAME_(n) set##n
#define FSO_MAKE_FUNC_SET_NAME(n) _FSO_MAKE_FUNC_SET_NAME_(n)
#define callFSOSetter(fieldName,args...) FSO_MAKE_FUNC_SET_NAME(fieldName)(args)

#if defined(__aarch64__)
    #define CALL_FUNC_GET(name)         \
        "adr    lr, return_from_func;"  \
        "call _get_"#name               \
        "return_from_func:"
    #define CALL_FUNC_SET(name)         \
        "adr    lr, return_from_func;"  \
        "call _set_"#name               \
        "return_from_func:"
#elif defined(__x86_64__)
    #define CALL_FUNC_GET(name)     "call _get_"#name";"
    #define CALL_FUNC_SET(name)     "call _set_"#name";"
#elif defined(__arm__)
#elif defined(__i386__)
#elif defined(__riscv)
    #if (__riscv_xlen == 64)
    #endif
#else
    #error Unknown Architecture
#endif

extern "C" void FSOGetterChecker();
extern "C" void FSOSetterChecker();

#define FSOGetter(fieldName,args...) FSO_MAKE_FUNC_GET_NAME(fieldName)(args) {  \
    __asm__ __volatile__(                                                       \
        CALL_FUNC_GET(fieldName)                                                \
    );                                                                          \
    /* If we do a check before CALL_FUNCTION, we have to keep all the registers \
       and restore after the check */                                           \
    FSOGetterChecker();                                                         \
};                                                                              \
extern "C" void FSO_MAKE_FUNC_GET_NAME_inside(fieldName)(args)

#define FSOSetter(fieldName,args...) FSO_MAKE_FUNC_SET_NAME(fieldName)(args) {  \
    __asm__ __volatile__(                                                       \
        CALL_FUNC_SET(fieldName)                                                \
    );                                                                          \
    /* If we do a check before CALL_FUNCTION, we have to keep all the registers \
       and restore after the check */                                           \
    FSOSetterChecker();                                                         \
};                                                                              \
extern "C" void FSO_MAKE_FUNC_SET_NAME_inside(fieldName)(args)

/*
void FSOGetter(PropertyName, const char *s)
{
    puts(s);
}

void FSOSetter(PropertyName, const char *s)
{
    puts(s);
}

int main()
{
    callFSOGetter(PropertyName, "===test main callFSOGetter====\n");
    callFSOSetter(PropertyName, "===test main callFSOSetter====\n");
}
*/

} // namespace como

#endif // __FSOGetterSetter_h__
