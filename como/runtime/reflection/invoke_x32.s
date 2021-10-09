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

// EXTERN_C ECode invoke(
//     /* [in] */ HANDLE func,
//     /* [in] */ Byte* params,
//     /* [in] */ Integer paramNum,
//     /* [in] */ Integer stackParamNum,
//     /* [in] */ struct ParameterInfo* paramInfos);

    .text
    .align 4;
    .global invoke;

invoke:
    push    %ebp;
    movl    %esp, %ebp;
    push    $0;                     // integral paramNum
    push    $0;                     // floating point paramNum

/* map of current memory and registers

    kernel space
    .
    user space
       +--------+ high address, stack  |
       |        |                      \/
       |        |
       |        | parameter#4, paramInfos    24(%ebp);  %esi
       |        | parameter#3, stackParamNum 20(%ebp);  %ecx
       |        | parameter#2, paramNum      16(%ebp);  %ebx
       |        | parameter#1, params        12(%ebp);  %eax
       |        | parameter#0, func           8(%ebp);
       |        | ret address                 4(%ebp);
%ebp-> +--------+ old %ebp                    0(%ebp);
       |   0    | local#0, integral paramNum          -4(%ebp);
       |   0    | local#1, floating point paramNum    -8(%ebp);
       |        |
%esp-> +--------+ low address
        ......

                current stack esp,                      %edi
                temp,                                   %edx
*/

    // alloc_stack
    cmpl    $0, %ecx;
    je      set_this;
    movl    $16, %eax;
    mull    %ecx;
    subl    %eax, %esp;
    movl    %esp, %edi;

set_this:
    movl    12(%ebp), %eax;         // params
    movl    16(%ebp), %ebx;         // paramNum
    movl    20(%ebp), %ecx;         // get "stackParamNum" value into %ecx
    movl    24(%ebp), %esi;         // paramInfos
    movl    (%eax), %edi;           // params[0]
    addl    $8, %eax;               // params + 8
    movl    $1, 0(%esp);            // next integral paramNum = 1
    subl    $1, %ebx;               // paramNum -= 1

set_params:
    testl   %ebx, %ebx;             // paramNum == 0 ?
    jz      call_func;
    movl    4(%esi), %eax;          // paramInfos->mNumberType
    cmpl    $0, %eax;               // paramInfos->mNumberType == NUMBER_TYPE_INTEGER ?
    je      set_integral_param;
    jmp     set_float_point_param;

set_integral_param:
    movl    (%ebp), %eax;           // next integral paramNum
    jmp     save_param_to_stack;

set_float_point_param:
    movl    4(%ebp), %eax;          // floating point paramNum
    jmp     save_param_to_stack;

save_param_to_stack:
    movl    (%esi), %eax;           // paramInfos->mSize
    movl    (%eax), %edx;           // params[0] -> %edx
    movl    %edx, (%edi);
    cmpl    $8, %eax;               // paramInfos->mSize == 8 ?
    je      stack_eight_bytes;
    addl    $8, %edi;               // next stack element
    addl    $4, %eax;               // next params
    subl    $1, %ebx;               // paramNum -= 1
    jmp     get_next_param;
stack_eight_bytes:
    addl    $8, %edi;               // next stack element
    addl    $8, %eax;               // next params
    subl    $1, %ebx;               // paramNum -= 1
    jmp     get_next_param;

get_next_param:
    addl    $20, %esi;              // jump to next paramInfos, sizeof(struct ParameterInfo) is #20
    movl    (%esi), %eax;           // paramInfos->mSize
    cmpl    $8, %eax;               // paramInfos->mSize == 8 ?
    je      eight_bytes_alignment;
    jmp     set_params;
eight_bytes_alignment:
    movl    %eax, %edx;             // %edx, temp, for test it
    andl    $7, %edx;
    cmpl    $0, %edx;
    je      do_not_adjust
    addl    $4, %eax;
do_not_adjust:
    jmp     set_params;

call_func:
    movl    8(%ebp), %eax;
    call    *%eax;

return:
    leave
    ret
