; -----------------------------------------------------------------------
;   
;   Copyright 2012-2013 by Gary Guo - All Rights Reserved
;
;   * All source code or binary file can only be used as personal
;     study or research, but can not be used for business.
;   * Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   * Redistributions in binary form must reproduce the above
;     copyright notice, this list of conditions and the following
;     disclaimer in the documentation and/or other materials provided
;     with the distribution.
;     
;     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
;     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
;     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
;     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
;     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
;     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
;     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
;     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
;     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; -----------------------------------------------------------------------
;     This file is the idt table and interrupt dealing part.
;     The path of this source code is kernel/interrupt.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

[BITS 32]

%include "boot/include/loader.inc"
%include "boot/include/protect.inc"
%include "const.inc"

[section .data]	; GDT and IDT

align 8

global IDTTable
global unknown_irq
global init_8259A

extern system_call
extern handlerTable

SEL_FLAT_C	equ		8

IDTTable:
GateDescriptor	DIVIDE_ERROR, SEL_FLAT_C, 0, DA_386IGate		;0
GateDescriptor	DEBUG_ERROR, SEL_FLAT_C, 0, DA_386IGate			;1
GateDescriptor	NMI_INTERRUPT, SEL_FLAT_C, 0, DA_386IGate		;2
GateDescriptor	BREAK_POINT, SEL_FLAT_C, 0, DA_386IGate + SDA_DPL3	;3
GateDescriptor	OVERFLOW_ERROR, SEL_FLAT_C, 0, DA_386IGate		;4
GateDescriptor	BOUND_CHECK, SEL_FLAT_C, 0, DA_386IGate			;5
GateDescriptor	INVALID_OPCODE, SEL_FLAT_C, 0, DA_386IGate		;6
GateDescriptor	NO_MATH_COPRO, SEL_FLAT_C, 0, DA_386IGate		;7
GateDescriptor	DOUBLE_FAULT, SEL_FLAT_C, 0, DA_386IGate		;8
GateDescriptor	COPRO_SEG_OVERRUN, SEL_FLAT_C, 0, DA_386IGate	;9
GateDescriptor	INVALID_TSS, SEL_FLAT_C, 0, DA_386IGate			;10
GateDescriptor	SEG_NOT_PRESENT, SEL_FLAT_C, 0, DA_386IGate		;11
GateDescriptor	STACK_SEG_ERROR, SEL_FLAT_C, 0, DA_386IGate		;12
GateDescriptor	GENERAL_PROTECTION, SEL_FLAT_C, 0, DA_386IGate	;13
GateDescriptor	PAGE_FAULT, SEL_FLAT_C, 0, DA_386IGate			;14
GateDescriptor	RESERVED_INTERRUPT, SEL_FLAT_C, 0, DA_386IGate	;15 Reserved
GateDescriptor	FLOATING_POINT_ERROR, SEL_FLAT_C, 0, DA_386IGate;16
GateDescriptor	ALIGN_CHECK, SEL_FLAT_C, 0, DA_386IGate			;17
GateDescriptor	MACHINE_CHECK, SEL_FLAT_C, 0, DA_386IGate		;18
GateDescriptor	SIMD_FLOATING_POINT, SEL_FLAT_C, 0, DA_386IGate	;19
GateDescriptor	VIRTUALIZATION_ERROR, SEL_FLAT_C, 0, DA_386IGate;20
%rep 31-21+1
GateDescriptor	RESERVED_INTERRUPT, SEL_FLAT_C, 0, DA_386IGate	;21-31 Reserved
%endrep
GateDescriptor	HWINT00, SEL_FLAT_C, 0, DA_386IGate	;IRQ 0
GateDescriptor	HWINT01, SEL_FLAT_C, 0, DA_386IGate	;IRQ 1
GateDescriptor	HWINT02, SEL_FLAT_C, 0, DA_386IGate	;IRQ 2
GateDescriptor	HWINT03, SEL_FLAT_C, 0, DA_386IGate	;IRQ 3
GateDescriptor	HWINT04, SEL_FLAT_C, 0, DA_386IGate	;IRQ 4
GateDescriptor	HWINT05, SEL_FLAT_C, 0, DA_386IGate	;IRQ 5
GateDescriptor	HWINT06, SEL_FLAT_C, 0, DA_386IGate	;IRQ 6
GateDescriptor	HWINT07, SEL_FLAT_C, 0, DA_386IGate	;IRQ 7
GateDescriptor	HWINT08, SEL_FLAT_C, 0, DA_386IGate	;IRQ 8
GateDescriptor	HWINT09, SEL_FLAT_C, 0, DA_386IGate	;IRQ 9
GateDescriptor	HWINT10, SEL_FLAT_C, 0, DA_386IGate	;IRQ10
GateDescriptor	HWINT11, SEL_FLAT_C, 0, DA_386IGate	;IRQ11
GateDescriptor	HWINT12, SEL_FLAT_C, 0, DA_386IGate	;IRQ12
GateDescriptor	HWINT13, SEL_FLAT_C, 0, DA_386IGate	;IRQ13
GateDescriptor	HWINT14, SEL_FLAT_C, 0, DA_386IGate	;IRQ14
GateDescriptor	HWINT15, SEL_FLAT_C, 0, DA_386IGate	;IRQ15
GateDescriptor	SYS_CALL, SEL_FLAT_C, 0, DA_386IGate + SDA_DPL3	;0x30
%rep 256-0x31
GateDescriptor	RESERVED_INTERRUPT, SEL_FLAT_C, 0, DA_386IGate	;0x30~ Reserved
%endrep
IDT_END:

align 4
IdtPtr:			IDTPointer IDTTable, IDT_END - IDTTable

[section .init_text]

extern cstart

start:
	cli
	
	mov		esp, 0xC0090000	; Set the stack
	
	finit
	lidt	[IdtPtr]
	
	jmp		cstart

[section .text]

global _start

extern exception_handler
extern page_fault
extern breakpoint_disp
extern save

_start:
	jmp		start
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
DIVIDE_ERROR			equ divide_error - $$ + KERNEL_ENTRY
DEBUG_ERROR				equ debug_error - $$ + KERNEL_ENTRY
NMI_INTERRUPT			equ nmi_interrupt - $$ + KERNEL_ENTRY
BREAK_POINT				equ break_point - $$ + KERNEL_ENTRY
OVERFLOW_ERROR			equ overflow_error - $$ + KERNEL_ENTRY
BOUND_CHECK				equ bound_check - $$ + KERNEL_ENTRY
INVALID_OPCODE			equ invalid_opcode - $$ + KERNEL_ENTRY
NO_MATH_COPRO			equ no_math_copro - $$ + KERNEL_ENTRY
DOUBLE_FAULT			equ double_fault - $$ + KERNEL_ENTRY
COPRO_SEG_OVERRUN		equ copro_seg_overrun - $$ + KERNEL_ENTRY
INVALID_TSS				equ invalid_tss - $$ + KERNEL_ENTRY
SEG_NOT_PRESENT			equ seg_not_present - $$ + KERNEL_ENTRY
STACK_SEG_ERROR			equ stack_seg_error - $$ + KERNEL_ENTRY
GENERAL_PROTECTION		equ general_protection - $$ + KERNEL_ENTRY
PAGE_FAULT				equ page_fault_hdr - $$ + KERNEL_ENTRY
FLOATING_POINT_ERROR	equ floating_point_error - $$ + KERNEL_ENTRY
ALIGN_CHECK				equ align_check - $$ + KERNEL_ENTRY
MACHINE_CHECK			equ machine_check - $$ + KERNEL_ENTRY
SIMD_FLOATING_POINT		equ simd_floating_point - $$ + KERNEL_ENTRY
VIRTUALIZATION_ERROR	equ virtualization_error - $$ + KERNEL_ENTRY
RESERVED_INTERRUPT		equ reserved_interrupt - $$ + KERNEL_ENTRY
HWINT00					equ hwint00 - $$ + KERNEL_ENTRY
HWINT01					equ hwint01 - $$ + KERNEL_ENTRY
HWINT02					equ hwint02 - $$ + KERNEL_ENTRY
HWINT03					equ hwint03 - $$ + KERNEL_ENTRY
HWINT04					equ hwint04 - $$ + KERNEL_ENTRY
HWINT05					equ hwint05 - $$ + KERNEL_ENTRY
HWINT06					equ hwint06 - $$ + KERNEL_ENTRY
HWINT07					equ hwint07 - $$ + KERNEL_ENTRY
HWINT08					equ hwint08 - $$ + KERNEL_ENTRY
HWINT09					equ hwint09 - $$ + KERNEL_ENTRY
HWINT10					equ hwint10 - $$ + KERNEL_ENTRY
HWINT11					equ hwint11 - $$ + KERNEL_ENTRY
HWINT12					equ hwint12 - $$ + KERNEL_ENTRY
HWINT13					equ hwint13 - $$ + KERNEL_ENTRY
HWINT14					equ hwint14 - $$ + KERNEL_ENTRY
SYS_CALL				equ sys_call - $$ + KERNEL_ENTRY
HWINT15					equ hwint15 - $$ + KERNEL_ENTRY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%macro NOCODE_EXCEPTION 2
%1:
	call	save
	push	-1
	push	%2
	call	[kernel_str+GDT_LENGTH*8+16+%2*4]
	add		esp, 4*2
	ret
%endmacro
%macro CODED_EXCEPTION 2
%1:
	pop		dword[esp-(4+8+1+1+1)*4]
	; 4+8+1 skip 4 Seg Regs, 8 GPRs, 1 `restart`,
	; another 1 skip the ret_addr
	; the other 1 skip the err_code which is going to pop out.
	call	save
	sub		esp, 4
	push	%2
	call	[kernel_str+GDT_LENGTH*8+16+%2*4]
	add		esp, 4*2
	ret
%endmacro

NOCODE_EXCEPTION	divide_error, 0
NOCODE_EXCEPTION	debug_error, 1
NOCODE_EXCEPTION	nmi_interrupt, 2
break_point:
	call	save
	call	breakpoint_disp
	ret
NOCODE_EXCEPTION	overflow_error, 4
NOCODE_EXCEPTION	bound_check, 5
NOCODE_EXCEPTION	invalid_opcode, 6
NOCODE_EXCEPTION	no_math_copro, 7
CODED_EXCEPTION		double_fault, 8
NOCODE_EXCEPTION	copro_seg_overrun, 9
CODED_EXCEPTION		invalid_tss, 10
CODED_EXCEPTION		seg_not_present, 11
CODED_EXCEPTION		stack_seg_error, 12
CODED_EXCEPTION		general_protection, 13
CODED_EXCEPTION		page_fault_hdr, 14
;15 is reserved
NOCODE_EXCEPTION	floating_point_error, 16
CODED_EXCEPTION		align_check, 17
NOCODE_EXCEPTION	machine_check, 18
NOCODE_EXCEPTION	simd_floating_point, 19
NOCODE_EXCEPTION	virtualization_error, 20
NOCODE_EXCEPTION	reserved_interrupt, -1
	
%macro HWINTHandler 1
hwint%1:
	call	save
	push	%1
	call	[(kernel_str+GDT_LENGTH*8+16+0x20*4)+%1*4]
	; kernel_str+GDTLENGTH*8+16+0x20*4=kernel_str.handlerTable2
	add		esp, 4
	mov		eax, 0xFEE000B0
	mov		dword[eax], 0
	ret	
%endmacro
HWINTHandler 00
HWINTHandler 01
HWINTHandler 02
HWINTHandler 03
HWINTHandler 04
HWINTHandler 05
HWINTHandler 06
HWINTHandler 07
HWINTHandler 08
HWINTHandler 09
HWINTHandler 10
HWINTHandler 11
HWINTHandler 12
HWINTHandler 13
HWINTHandler 14
HWINTHandler 15
	
sys_call:
	call save
	push ebx
	push ecx
	push edx
	push eax
	call [(kernel_str+GDT_LENGTH*8+16)+0x30*4]
	add  esp, 16
	ret
	
