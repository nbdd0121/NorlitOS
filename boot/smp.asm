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
;     This file is the smp loader of northern lights operating system.
;     The path of this source code is boot/smp.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

%include "loader.inc"

org SMP_OFFSET

[Section Bit16]
[BITS    16]

jmp LABEL_START

%macro DEFINE_STRING 2
%1.0 db  %2, 0
%1   equ %1.0 + LOADER_PHYADDR
%endmacro

%include "protect.inc"

; The entry point of this program
LABEL_START:
	mov     ax, cs
	mov     ds, ax
	mov     es, ax
	
	cli

	lgdt    [GdtPtr]

	in      al, 92h
	or      al, 010b
	out     92h, al

	mov     eax, cr0
	bts     eax, 0
	mov     cr0, eax

	jmp     dword SEL_FLAT_C:(SMP_PHYADDR+LABEL_START_PM)

; Data
align 64

GDT_EMPTY:      SegmentDescriptor 0,0,0							;空描述符
GDT_FLAT_C:     SegmentDescriptor 0,0xFFFFF,SDA_FLAT_C			;平坦系统代码段
GDT_FLAT_D:     SegmentDescriptor 0,0xFFFFF,SDA_FLAT_D			;平坦系统数据段

GDT_END:

align 16
GdtPtr:         GDTPointer GDT_EMPTY + SMP_PHYADDR, GDT_END - GDT_EMPTY

align 16
GdtPtr2:         GDTPointer GDT_EMPTY + SMP_PHYADDR + 0xC0000000, GDT_END - GDT_EMPTY

SEL_FLAT_C      EQU GDT_FLAT_C - GDT_EMPTY
SEL_FLAT_D      EQU GDT_FLAT_D - GDT_EMPTY

[Section Bit32]
[BITS 32]

LABEL_START_PM:
	cli
	mov     ax, SEL_FLAT_D
	mov     gs, ax
	mov     fs, ax
	mov     ds, ax
	mov     es, ax
	mov     ss, ax
	
	mov     eax, PDEAddr
	mov     cr3, eax
	
	mov     eax, cr0
	or      eax, 0x80000000	; CR0.PG=1, 分页机制使能
	mov     cr0, eax
	
	jmp     pageafter

pageafter:
	nop

	lgdt	[GdtPtr2 + SMP_PHYADDR + 0xC0000000]

	mov		eax, LockAddr
	call	spin_lock
	
	jmp		SEL_FLAT_C:KERNEL_ENTRY
	
spin_lock:
	mov		edx, eax
.spin:
	mov		eax, 1
	xchg	eax, [edx]
	or		eax, eax
    jz		.ret
	pause
	jmp		.spin
.ret:
    ret


