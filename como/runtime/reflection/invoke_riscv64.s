#=========================================================================
# Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=========================================================================


# Declare a function called name, doesn't set up $gp.
.macro ENTRY_NO_GP_CUSTOM_CFA name, cfa_offset
    .type \name, %function
    .global \name
    # Cache alignment for function entry.
    .balign 16
\name:
    .cfi_startproc
     # Ensure we get a sane starting CFA.
    .cfi_def_cfa sp, \cfa_offset
.endm

# Declare a function called name, doesn't set up $gp.
.macro ENTRY_NO_GP name
    ENTRY_NO_GP_CUSTOM_CFA \name, 0
.endm

# Declare a function called name, sets up $gp.
# This macro modifies t8.
.macro ENTRY name
    ENTRY_NO_GP \name
    # Set up $gp and store the previous $gp value to $t8. It will be pushed to the
    # stack after the frame has been constructed.
    # FIXME: T-HEAD, Need check here in future.
    # .cpsetup $t9, $t8, \name
    # Declare a local convenience label to be branched to when $gp is already set up.
.L\name\()_gp_set:
.endm

.macro END name
    .cfi_endproc
    .size \name, .-\name
.endm

.macro UNIMPLEMENTED name
    ENTRY \name
    break
    break
    END \name
.endm


# EXTERN_C ECode invoke(
#     /* [in] */ HANDLE func,
#     /* [in] */ Byte* params,
#     /* [in] */ Integer paramNum,
#     /* [in] */ Integer stackParamNum,
#     /* [in] */ struct ParameterInfo* paramInfos);

# x1:   subroutine return address
# x10:  holds "func" value
# x11:  holds "params" value
# w12:  holds "paramNum" value
# w13:  holds "stackParamNum" value
# x14:  holds "paramInfos" value

ENTRY_NO_GP invoke
    addi    sp, sp, -16
    sd      fp, 8(sp)           # x8, s0 or fp, saved register 0 or frame pointer
    sd      ra, 16(sp)          # x1, ra, return address for jumps
    move    fp, sp
    tail    main
main:
    #beqz    rs, x0, call_func
END invoke