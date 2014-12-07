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
;     This file is the boot sector of northern lights operating system.
;     The path of this source code is boot/boot.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

org 07c00h

[BITS    16]

BaseOfStack: ; Stack

%include    "loader.inc"

; The entry point of this program
LABEL_START:
	mov     ax, 0
	mov     ds, ax
	mov     es, ax
	mov     ss, ax
	mov     sp, BaseOfStack
	
	mov     si, String
	call    DispStr

	mov		eax, [LBA1]
	cmp		byte[TYPE1], 0x6E
	je		start
	cmp		byte[TYPE1], 5
	je		extended
	
	mov		eax, [LBA2]
	cmp		byte[TYPE2], 0x6E
	je		start
	cmp		byte[TYPE2], 5
	je		extended
	
	mov		eax, [LBA3]
	cmp		byte[TYPE3], 0x6E
	je		start
	cmp		byte[TYPE3], 5
	je		extended
	
	mov		eax, [LBA4]
	cmp		byte[TYPE4], 0x6E
	je		start
	cmp		byte[TYPE4], 5
	je		extended
	
	mov		si, StrCnt
	call	DispStr
	
	cli
	hlt

extended:
	push	eax
.0:
	push	eax
	mov		si, StrExt
	call	DispStr
	
	mov     ax, HDBOOT_SEGMENT	; Target Address for INT 13h
	mov     es, ax				; Argument for DiskRead::ES

	pop		eax
	push	eax
	mov     cl, 1				; Argument for DiskRead::Size
	
	mov     bx, HDBOOT_OFFSET		; Argument for DiskRead::BX
	call    DiskRead
	
	pop		eax
	push	eax
	add		eax, [es:446+8]
	cmp		byte [es:446+4], 0x6E
	je		start
	
	pop		eax
	pop		eax
	push	eax
	add		eax, [es:446+16+8]
	cmp		byte [es:446+16+4], 5
	je		.0
	
	mov		si, StrCnt
	call	DispStr

	cli
	hlt
	
start:
	push	eax

	mov     ax, LOADER_SEGMENT	; Target Address for INT 13h
	mov     es, ax				; Argument for DiskRead::ES

	pop		eax
	push	eax
	add     eax, LOADER_FILE_LBA	; Argument for DiskRead::LBA Address
	mov     cl, LOADER_SIZE_SECT	; Argument for DiskRead::Size
	
	mov     bx, LOADER_OFFSET		; Argument for DiskRead::BX
	call    DiskRead
	
	mov     ax, KERNELFILE_SEGMENT	; Target Address for INT 13h
	mov     es, ax					; Argument for DiskRead::ES

	pop		eax
	push	eax
	add     eax, KERNEL_FILE_LBA	; Argument for DiskRead::LBA Address
	mov     cl, KERNEL_SIZE_SECT	; Argument for DiskRead::Size
	
	mov     bx, KERNELFILE_OFFSET	; Argument for DiskRead::BX
	call    DiskRead
	
	mov     ax, SMP_SEGMENT			; Target Address for INT 13h
	mov     es, ax					; Argument for DiskRead::ES

	pop		eax
	add     eax, SMP_FILE_LBA		; Argument for DiskRead::LBA Address
	mov     cl, SMP_SIZE_SECT		; Argument for DiskRead::Size
	
	mov     bx, SMP_OFFSET			; Argument for DiskRead::BX
	call    DiskRead
	
	mov		ax, HDBOOT_SEGMENT
	mov		es, ax
	mov		byte[es:HDBOOT_OFFSET], 0
	
	jmp     LOADER_SEGMENT:LOADER_OFFSET

	cli		; Should never reach here
	hlt

; Libraries

%include "libHD.inc"

; Data

String db "Booting HD",0
StrExt db "`",0
StrCnt db " :-( No Norlit OS Installed.",0

times 446-($-$$) db 0	; Fill util 446Bytes

; MBR
times 4 db 0
TYPE1 db 0
times 3 db 0
LBA1 dd 0
dd 0

times 4 db 0
TYPE2 db 0
times 3 db 0
LBA2 dd 0
dd 0

times 4 db 0
TYPE3 db 0
times 3 db 0
LBA3 dd 0
dd 0

times 4 db 0
TYPE4 db 0
times 3 db 0
LBA4 dd 0
dd 0

times 510-($-$$) db 0	; Fill until 510Bytes
dw    0xAA55	; Make the sector valid
