//=========================================================================
// Copyright (C) 2023 The C++ Component Model(COMO) Open Source Project
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

#ifndef __INT64_H__
#define __INT64_H__

#include <stdbool.h>
#include <stdint.h>

namespace como {

#pragma pack(1)

typedef union tagInt64 {
    bool            bVal;
    int8_t          i8Val;      // == i8.i8_7
    int16_t         i16Val;     // == i16.i16_low_low
    int32_t         i32Val;     // == i32.i32_low
    int64_t         i64Val;
    float           floatVal;
    double          doubleVal;
    void           *pVal;

    // Low32 High32
    struct {
        int i32_low;
        int i32_high;
    } i32;

    // Low16 High16 Low16 High16
    // `----\/----/ `----\/----/
    //     Low32       High32
    struct {
        short i16_low_low;
        short i16_low_high;
        short i16_high_low;
        short i16_high_high;
    } i16;

    // char7 char6 char5 char4 char3 char2 char1 char0
    struct {
        char i8_7;
        char i8_6;
        char i8_5;
        char i8_4;
        char i8_3;
        char i8_2;
        char i8_1;
        char i8_0;
    } i8;
} Int64;

#pragma pack()

} // namespace como

#endif // __INT64_H__
