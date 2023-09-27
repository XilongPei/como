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

namespace como {

#pragma pack(1)
typedef union tagInt64 {
    bool            bVal;
    short           shortVal;
    long long int   i64Val;
    double          doubleVal;
    char           *charpVal;
    struct {
        int i32_low;
        int i32_high;
    } i32;
    struct {
        short i16_low_low;
        short i16_low_high;
        short i16_high_low;
        short i16_high_high;
    } i16;
    struct {
        char chr7;
        char chr6;
        char chr5;
        char chr4;
        char chr3;
        char chr2;
        char chr1;
        char chr0;
    } chr;
} Int64;
#pragma pack()

} // namespace como

#endif // __INT64_H__
