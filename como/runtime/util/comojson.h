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

#ifndef __ComoJSON_H__
#define __ComoJSON_H__

#include <pthread.h>
#include <setjmp.h>
#include <errno.h>
#include "comotypes.h"
#include "comoref.h"
#include "comolog.h"

#if 0  // some codes in json.hpp
const_reference at(size_type idx) const
{
    // at only works for arrays
    if (JSON_HEDLEY_LIKELY(is_array()))
    {
        JSON_TRY_USER
        {
            return m_value.array->at(idx);
        }
        JSON_CATCH_USER (std::out_of_range&)
        {
            // create better exception explanation
            JSON_THROW(out_of_range::create(401, detail::concat("array index ", std::to_string(idx), " is out of range"), this));
        }
    }
    else
    {
        JSON_THROW_USER(type_error::create(304, detail::concat("cannot use at() with ", type_name()), this));
    }
}
#endif

#define MAX_EXCEPTION_NESTING_LEVEL  16
static __thread int     _exn_handler_idx = -1;
static __thread jmp_buf _exn_handlers[MAX_EXCEPTION_NESTING_LEVEL];

// In a closure environment, there may be n `try ...catch ...`,  so this variable
// has been defined as a global variable.
static __thread int __JSON_EXCEPTION__;

//#define std::out_of_range(aaa) ::out_of_range::create(408, aaa)

#define JSON_TRY_USER                                                          \
    if (_exn_handler_idx < MAX_EXCEPTION_NESTING_LEVEL)                        \
        _exn_handler_idx++;                                                    \
    if (! (__JSON_EXCEPTION__ = setjmp(_exn_handlers[_exn_handler_idx])))      \

#define JSON_CATCH_USER(excep)                                                 \
    if (_exn_handler_idx--, __JSON_EXCEPTION__)                                \

#define JSON_THROW_USER(excep)                                                 \
    if (std::is_base_of<typeof(excep), nlohmann::detail::exception>::value)    \
        errno = ((nlohmann::detail::exception)excep).id;                       \
    Logger::E("JSON", excep.what());                                           \
    longjmp(_exn_handlers[_exn_handler_idx], _exn_handler_idx+1);

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#endif // __ComoJSON_H__
