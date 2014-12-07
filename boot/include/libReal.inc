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

;=====================================================
; DiskReset(char device);复位驱动器
;-----------------------------------------------------
; Exit:
;	- CFLAGS.CF -> 错误标志
;	- AH -> 错误代码
; registers changed:
;	- AH, DL
DiskReset:
	xor     ah, ah
	mov     dl, 0
	int     13h
	ret

;=====================================================
; DiskRead(char address, char num, void* buffer);从驱动器读取数据
;-----------------------------------------------------
; Entry:
;	- AX -> 扇区号
;	- CX -> 扇区总数
;	- ES:BX -> 缓冲区
; Exit:
;	- CFLAGS.CF -> 错误标志
;	- AH -> 错误代码
;	- AL -> 扇区总数
; registers changed:
;	- AX, DX, CX, BX
DiskRead:
	push    ax
	push    bx
	push    cx		; 保存着循环次数，先保存
	call    LBA2CHS
	mov     ax, 0201h
	mov     dl, 0
	int     13h		; 读一个扇区

	mov     si, Point
	call    DispStr	; 可视化，打个点
	pop     cx		; 恢复循环次数
	pop     bx
	add     bx, 512
	jnc     .next
	mov     ax, es
	add     ax, 0x1000
	mov     es, ax
.next:
	pop     ax
	inc     ax
	loop	DiskRead
	ret

Point  db ".",0
	
;=====================================================
; LBA2CHS(char id);将LBA寻址转化为CHS寻址
;-----------------------------------------------------
; Entry:
;	- AX -> 扇区号
; Exit:
;	- CL -> 扇区号
;	- CH -> 柱面号
;	- DH -> 磁头号
; registers changed:
;	- AX, CX, DH
LBA2CHS:
	push    bx		; 同理
	
	mov     bx, SectorPerTrack		; 每磁道有18个扇区
	div     bl
	
	inc     ah		; 扇区号=余数+1
	mov     cl, ah	; 填充INT 13h所需参数
	xor     ah, ah	; 将ax前面用0填充
	
	mov     bx, TrackPerCylinder	; 每柱面有两个磁道
	div     bl
	mov     ch, al	; 这是柱面
	mov     dh, ah	; 这是磁头
	pop     bx		; BX已经用完了,恢复
	ret
	