#=========================================================================
# Copyright (C) 2025 The C++ Component Model(COMO) Open Source Project
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
    addi    sp, sp, -48
    sd      ra, 40(sp)
    sd      fp, 32(sp)
    sd      s1, 24(sp)
    addi    fp, sp, 48

    mv      t0, a0              # func
    mv      t1, a1              # params
    mv      t2, a2              # paramNum
    mv      t3, a3              # stackParamNum
    mv      t4, a4              # paramInfos

    li      t5, 0               # next integral paramNum
    li      t6, 0               # next floating point paramNum

    # alloc_stack
    beqz    t3, set_this
    li      a7, 16
    mul     t3, t3, a7
    sub     sp, sp, t3
    mv      s1, sp

set_this:
    ld      a0, 0(t1)
    addi    t1, t1, 8
    li      t5, 1
    addi    t2, t2, -1

set_params:
    beqz    t2, call_func
    lw      a7, 4(t4)           # paramInfos->mNumberType
    beqz    a7, set_integral_param
    j       set_float_point_param

get_next_param:
    addi    t4, t4, 20
    lw      a7, 0(t4)
    li      a6, 8
    beq     a7, a6, eight_bytes_alignment
    j       set_params

eight_bytes_alignment:
    andi    a7, t1, 7
    beqz    a7, do_not_adjust
    addi    t1, t1, 4
do_not_adjust:
    j       set_params

set_integral_param:
    li      a7, 1
    beq     t5, a7, set_reg_a1
    li      a7, 2
    beq     t5, a7, set_reg_a2
    li      a7, 3
    beq     t5, a7, set_reg_a3
    li      a7, 4
    beq     t5, a7, set_reg_a4
    li      a7, 5
    beq     t5, a7, set_reg_a5
    li      a7, 6
    beq     t5, a7, set_reg_a6
    li      a7, 7
    beq     t5, a7, set_reg_a7
    j       save_param_to_stack

set_reg_a1:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a1_w
    ld      a1, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a1_end
set_reg_a1_w:
    lw      a1, 0(t1)
    addi    t1, t1, 4
set_reg_a1_end:
    li      t5, 2
    addi    t2, t2, -1
    j       get_next_param

set_reg_a2:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a2_w
    ld      a2, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a2_end
set_reg_a2_w:
    lw      a2, 0(t1)
    addi    t1, t1, 4
set_reg_a2_end:
    li      t5, 3
    addi    t2, t2, -1
    j       get_next_param

set_reg_a3:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a3_w
    ld      a3, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a3_end
set_reg_a3_w:
    lw      a3, 0(t1)
    addi    t1, t1, 4
set_reg_a3_end:
    li      t5, 4
    addi    t2, t2, -1
    j       get_next_param

set_reg_a4:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a4_w
    ld      a4, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a4_end
set_reg_a4_w:
    lw      a4, 0(t1)
    addi    t1, t1, 4
set_reg_a4_end:
    li      t5, 5
    addi    t2, t2, -1
    j       get_next_param

set_reg_a5:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a5_w
    ld      a5, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a5_end
set_reg_a5_w:
    lw      a5, 0(t1)
    addi    t1, t1, 4
set_reg_a5_end:
    li      t5, 6
    addi    t2, t2, -1
    j       get_next_param

set_reg_a6:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a6_w
    ld      a6, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a6_end
set_reg_a6_w:
    lw      a6, 0(t1)
    addi    t1, t1, 4
set_reg_a6_end:
    li      t5, 7
    addi    t2, t2, -1
    j       get_next_param

set_reg_a7:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_a7_w
    ld      a7, 0(t1)
    addi    t1, t1, 8
    j       set_reg_a7_end
set_reg_a7_w:
    lw      a7, 0(t1)
    addi    t1, t1, 4
set_reg_a7_end:
    li      t5, 8
    addi    t2, t2, -1
    j       get_next_param

set_float_point_param:
    li      a7, 0
    beq     t6, a7, set_reg_fa0
    li      a7, 1
    beq     t6, a7, set_reg_fa1
    li      a7, 2
    beq     t6, a7, set_reg_fa2
    li      a7, 3
    beq     t6, a7, set_reg_fa3
    li      a7, 4
    beq     t6, a7, set_reg_fa4
    li      a7, 5
    beq     t6, a7, set_reg_fa5
    li      a7, 6
    beq     t6, a7, set_reg_fa6
    li      a7, 7
    beq     t6, a7, set_reg_fa7
    j       save_param_to_stack

set_reg_fa0:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa0_s
    fld     fa0, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa0_end
set_reg_fa0_s:
    flw     fa0, 0(t1)
    addi    t1, t1, 4
set_reg_fa0_end:
    li      t6, 1
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa1:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa1_s
    fld     fa1, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa1_end
set_reg_fa1_s:
    flw     fa1, 0(t1)
    addi    t1, t1, 4
set_reg_fa1_end:
    li      t6, 2
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa2:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa2_s
    fld     fa2, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa2_end
set_reg_fa2_s:
    flw     fa2, 0(t1)
    addi    t1, t1, 4
set_reg_fa2_end:
    li      t6, 3
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa3:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa3_s
    fld     fa3, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa3_end
set_reg_fa3_s:
    flw     fa3, 0(t1)
    addi    t1, t1, 4
set_reg_fa3_end:
    li      t6, 4
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa4:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa4_s
    fld     fa4, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa4_end
set_reg_fa4_s:
    flw     fa4, 0(t1)
    addi    t1, t1, 4
set_reg_fa4_end:
    li      t6, 5
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa5:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa5_s
    fld     fa5, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa5_end
set_reg_fa5_s:
    flw     fa5, 0(t1)
    addi    t1, t1, 4
set_reg_fa5_end:
    li      t6, 6
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa6:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa6_s
    fld     fa6, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa6_end
set_reg_fa6_s:
    flw     fa6, 0(t1)
    addi    t1, t1, 4
set_reg_fa6_end:
    li      t6, 7
    addi    t2, t2, -1
    j       get_next_param

set_reg_fa7:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, set_reg_fa7_s
    fld     fa7, 0(t1)
    addi    t1, t1, 8
    j       set_reg_fa7_end
set_reg_fa7_s:
    flw     fa7, 0(t1)
    addi    t1, t1, 4
set_reg_fa7_end:
    li      t6, 8
    addi    t2, t2, -1
    j       get_next_param

save_param_to_stack:
    lw      a7, 0(t4)
    li      a6, 4
    ble     a7, a6, stack_four_bytes
    
    andi    a7, s1, 7
    beqz    a7, stack_no_adjust
    addi    s1, s1, 4
stack_no_adjust:
    ld      a7, 0(t1)
    sd      a7, 0(s1)
    addi    t1, t1, 8
    addi    s1, s1, 8
    addi    t2, t2, -1
    j       get_next_param

stack_four_bytes:
    lw      a7, 0(t1)
    sw      a7, 0(s1)
    addi    t1, t1, 4
    addi    s1, s1, 4
    addi    t2, t2, -1
    j       get_next_param

call_func:
    jalr    t0

    addi    sp, fp, -48
    ld      s1, 24(sp)
    ld      fp, 32(sp)
    ld      ra, 40(sp)
    addi    sp, sp, 48
    ret

END invoke
