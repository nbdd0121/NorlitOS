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
;     This file is the including file for loader.asm and boot.asm
;     The path of this source code is boot/include/libReal.inc
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

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

DiskAddressPacket:
	db 0x10
	db 0
count db 2 ; TRANSFERED
	db 0
offset dw 0 ; OFFSET
segm dw 0 ; SEG
lbaLow dd 0 ; LBA Low
	dd 0

;=====================================================
; DiskRead(char address, char num, void* buffer);从驱动器读取数据
;-----------------------------------------------------
; Entry:
;	- EAX -> 扇区号
;	- CX -> 扇区总数
;	- ES:BX -> 缓冲区
; Exit:
;	- CFLAGS.CF -> 错误标志
;	- AH -> 错误代码
;	- AL -> 扇区总数
; registers changed:
;	- AX, DX, CX, BX
DiskRead:
	mov		[lbaLow], eax
	mov		[count] , cl
	mov		ax      , es
	mov		[segm]  , ax
	mov		[offset], bx
	mov		si, DiskAddressPacket
	mov		ah, 42h
	mov		dl, 80h
	
	int		13h
	mov     si, Point
	call    DispStr	; 可视化，打个点
	ret

Point  db ".",0
