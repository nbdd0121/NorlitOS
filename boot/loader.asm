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
;     This file is the loader of northern lights operating system.
;     The path of this source code is boot/loader.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

%include "loader.inc"

org LOADER_OFFSET

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
	mov     si, STR_LOADING.0
	call    DispStr

;--------Init VESA Through BIOS Extension----------
InitVBE:
	mov		ax, VESAAddrSeg
	mov     es, ax
	mov		di, VESASign
	mov		ax, 4F00h
	int		10h
	cmp		byte[di+0x5], 0x2
	jb		.fail
	
	; Let ds:si points to the first mode
	mov		si, [es:VESAModes]
	mov		ax, [es:VESAModes+2]
	mov		ds, ax
	mov		di, VESAEnd
	xor		dx, dx
.loop:
	mov		cx, [si]
	cmp		cx, 0FFFFh
	je		.ok
	mov		ax, 4F01h
	int		10h
	mov		[di+0x0C], cx
	add		si, 2
	test		byte[di], 0b10000000
	jz		.loop
	; Check if the condition was satisfied
	; Need to be direct color, not palette or packed
	cmp		byte[di+0x1B], 6
	jne		.loop
	; Filter out useless resolutions
	cmp		word[di+0x14], 600
	jb		.loop
	; The following checks make sure that every
	; pixel was 24 bit or 32 bit wide, and coded as RGB.
	cmp		byte[di+0x1F], 8
	jne		.loop
	cmp		byte[di+0x20], 16
	jne		.loop
	cmp		byte[di+0x21], 8
	jne		.loop
	cmp		byte[di+0x22], 8
	jne		.loop
	cmp		byte[di+0x23], 8
	jne		.loop
	cmp		byte[di+0x24], 0
	jne		.loop
	
	add		di, 50
	inc		dx
	jmp		.loop
.dispError:
	mov     si, STR_NOVBE.0
	call    DispStr
	cli
	hlt
.fail:
	xor		dx, dx
.ok:
	; Restore DS
	mov		ax, cs
	mov		ds, ax
	
	; Wrtte VESAModeCount and check if count==0
	mov		[es:VESAModeCount], dx
	or		dx, dx
	jz		.dispError
	
	; Get the mode number out
	mov		bx, [es:VESAEnd+0xC]
	or		bx, 0100000000000000b ; Set LFB
	mov		ax, 4F02h
	int		10h
.end:
	
;--------------------------------------------------
	
; -------- Read Address Map From BIOS --------------
ReadMap:
	mov     ax, BootParamAddrSeg
	mov     es, ax
	mov     di, ARDSAddrOffset
	
	mov     dword[es:ARDSNum],0
	mov     ebx, 0
.loop:
	mov     edx, 0x534D4150
	mov     eax, 0xE820
	mov     ecx, 20
	int     15h
	jc      .fail
	add     di, 20
	inc     dword[es:ARDSNum]
	or      ebx, ebx
	jnz     .loop
	jmp     .ok
.fail:
	mov     si, STR_FAIL	;显示出错信息
	call    DispStr
	cli						;停止系统运行
	hlt
.ok:
;--------------------------------------------------	
	cli

	lgdt    [GdtPtr]

	in      al, 92h
	or      al, 010b
	out     92h, al

	mov     eax, cr0
	bts     eax, 0
	mov     cr0, eax

	jmp     dword SEL_FLAT_C:(LOADER_PHYADDR+LABEL_START_PM)

;=====================================================
; DispStr(char* addr);显示字符串
;-----------------------------------------------------
; Entry:
;	- DS:SI -> 要显示的字符串
; registers changed:
;	- BX, SI, AX
DispStr:
	mov     bx, 0000Fh
	mov     ah, 0Eh
.loop:
	mov     al, [si]
	inc     si
	or      al, al
	jz      .end
	int     10h
	jmp     .loop
.end:
	ret

; Data
align 64

GDT_EMPTY:      SegmentDescriptor 0,0,0							;空描述符
GDT_FLAT_C:     SegmentDescriptor 0,0xFFFFF,SDA_FLAT_C			;平坦系统代码段
GDT_FLAT_D:     SegmentDescriptor 0,0xFFFFF,SDA_FLAT_D			;平坦系统数据段

GDT_END:

GdtPtr:         GDTPointer GDT_EMPTY + LOADER_PHYADDR, GDT_END - GDT_EMPTY

SEL_FLAT_C      EQU GDT_FLAT_C - GDT_EMPTY
SEL_FLAT_D      EQU GDT_FLAT_D - GDT_EMPTY

DEFINE_STRING	STR_LOADING, `\r\nLoading`
DEFINE_STRING	STR_FAIL, ":-( Your CPU is too old to run Norlit OS."
DEFINE_STRING	STR_NOVBE, `\r\n:-( Your Video Card did not support VBE2.0 or resolution is too low.`

DispPos dd (80*0+0)*2+0xB8000

[Section Bit32]
[BITS 32]

%macro puts 1
mov  esi, %1
call DispStrPM
%endmacro

LABEL_START_PM:
	cli
	mov     ax, SEL_FLAT_D
	mov     gs, ax
	mov     fs, ax
	mov     ds, ax
	mov     es, ax
	mov     ss, ax

	; Check for EFLAGS.ID to check if CPUID is supported
	pushfd
	mov     eax, [esp]
	bts     dword[esp], 21
	popfd
	pushfd
	pop     ebx
	cmp     eax, ebx
	je		too_old
	
	; Check for CPUID.01h:EDX[bit 5] to check if MSR is supported
	xor		eax, eax
	inc		eax
	cpuid
	bt		edx, 5
	jnc		too_old
	
	; Check for CPUID.01h:EDX[bit 9] to check if APIC is supported
	bt		edx, 9
	jnc		too_old
	
	jmp     PagingOn

too_old:
	mov     esi, STR_FAIL
	call    DispStrPM
	cli
	hlt
	
pageafter:
	mov     esi, KERNELFILE_PHYADDR
	mov     edi, KERNEL_ENTRY
	mov     ecx, KERNEL_SIZE_SECT*512
	call    memcpy
	
	xor		eax, eax
	jmp		SEL_FLAT_C:KERNEL_ENTRY
; Library

%include "lib.inc"

; Data
DispPosPM equ DispPos+LOADER_PHYADDR
