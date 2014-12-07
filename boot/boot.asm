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

BaseOfStack: ; 栈向低处生长

jmp LABEL_START

%include    "loader.inc"

; The entry point of this program
LABEL_START:
	mov     ax, cs
	mov     ds, ax
	mov     es, ax
	mov     ss, ax
	mov     sp, BaseOfStack

	mov     si, String
	call    DispStr

	call    DiskReset	; Reset Device 0

	mov     ax, LOADER_SEGMENT	; Target Address for INT 13h
	mov     es, ax				; Argument for DiskRead::ES

	mov     ax, LOADER_FILE_LBA	; Argument for DiskRead::LBA Address
	mov     cl, LOADER_SIZE_SECT; Argument for DiskRead::Size
	
	mov     bx, LOADER_OFFSET	; Argument for DiskRead::BX
	call    DiskRead
	
	
	mov     ax, KERNELFILE_SEGMENT	; Target Address for INT 13h
	mov     es, ax					; Argument for DiskRead::ES

	mov     ax, KERNEL_FILE_LBA		; Argument for DiskRead::LBA Address
	mov     cl, KERNEL_SIZE_SECT	; Argument for DiskRead::Size
	
	mov     bx, KERNELFILE_OFFSET	; Argument for DiskRead::BX
	call    DiskRead
	
	mov     ax, SMP_SEGMENT			; Target Address for INT 13h
	mov     es, ax					; Argument for DiskRead::ES

	mov     ax, SMP_FILE_LBA		; Argument for DiskRead::LBA Address
	mov     cl, SMP_SIZE_SECT		; Argument for DiskRead::Size
	
	mov     bx, SMP_OFFSET			; Argument for DiskRead::BX
	call    DiskRead
	
	mov     ax, HDBOOT_SEGMENT		; Target Address for INT 13h
	mov     es, ax					; Argument for DiskRead::ES

	mov     ax, HDBOOT_FILE_LBA		; Argument for DiskRead::LBA Address
	mov     cl, HDBOOT_SIZE_SECT	; Argument for DiskRead::Size
	
	mov     bx, HDBOOT_OFFSET			; Argument for DiskRead::BX
	call    DiskRead
	
	jmp     LOADER_SEGMENT:LOADER_OFFSET

	cli		; Should never reach here
	hlt

; Libraries

%include "libReal.inc"

; Data

String db "Booting",0

times 446-($-$$) db 0	; 填充到446字节
times 510-($-$$) db 0	; 填充到510字节
dw    0xAA55	; 由于编译器是小字节序，所以55AA变成AA55
times 1474560-($-$$) db 0	; 填充至软盘大小,生成的文件就可以直接当做软盘镜像
