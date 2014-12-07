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
;     This file is the including file for loader.asm
;     The path of this source code is boot/include/lib.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

;=====================================================
; DispStrPM(char* addr);显示字符串 32位
;-----------------------------------------------------
; Entry:
;	- ESI -> 要显示的字符串
; registers changed:
;	- ESI, EAX
DispStrPM:
	push    ebp

	mov     ebp, [DispPosPM]	; 获得当前显示位置
	mov     ah, 0Fh				; 字体颜色
.loop:
	mov     al, [esi]			; 取1字节
	inc     esi					; 指针移动到下一字节
	or      al, al				; if(al==0)?
	jz      .end				; 退出循环
	cmp     al, 13				; 回车？
	je      .13
	cmp     al, 10				; 换行？
	je      .10
	mov     [ebp], ax			; 填充字符
	inc     ebp
	inc     ebp
	jmp     .loop
.13:
	push    bx
	
	mov     eax, ebp
	sub     eax, 0xB8000
	mov     bl, 160
	div     bl
	movzx   eax, ah
	sub     ebp, eax
	
	pop     bx
	mov     ah, 0Fh
	jmp     .loop
.10:
	add     ebp, 160
	jmp     .loop
.end:
	mov     [DispPosPM], ebp
	pop     ebp
	ret

;=====================================================
; memcpy(void* dest,void* src,int size);内存复制 32位
;-----------------------------------------------------
; Entry:
;	- ESI -> 源	  (4字节对齐)
;	- EDI -> 目标 (4字节对齐)
;	- ECX -> 大小 (4的正整数倍数)
; registers changed:
;	- ESI, EDI, ECX, EAX
memcpy:
	mov     eax, [esi]
	mov    	[edi], eax
	add     esi, 4
	add     edi, 4
	sub     ecx, 4
	or      ecx,ecx
	jnz     memcpy
.ret:
	ret

;=====================================================
; PagingOn();分页机制使能（2级）只映射了4MB以下内存
;-----------------------------------------------------
; registers changed:
;	- EAX, EBX, ECX
PagingOn:
	mov     ebx, PTEAddr
	mov     eax, 00000111b
	mov     ecx, 1024*4
.loop:
	mov     [ebx], eax
	add     ebx, 4
	add     eax, 0x1000
	loop    .loop
	
	mov     ebx, PDEAddr
	mov     eax, PTEAddr+00000111b
	mov     ecx, 1024/4
.loop1:
	mov     [ebx], eax
	add		eax, 0x1000
	mov		[ebx+4], eax
	add		eax, 0x1000
	mov		[ebx+8], eax
	add		eax, 0x1000
	mov		[ebx+12], eax
	add     ebx, 16
	sub		eax, 0x3000
	loop    .loop1
	
	mov     eax, PDEAddr

;=====================================================
; ProtectModePagingOn();分页机制启动
;-----------------------------------------------------
; registers changed:
;	- EAX
ProtectModePagingOn:
	mov     cr3, eax
	
	mov     eax, cr0
	or      eax, 0x80000000	; CR0.PG=1, 分页机制使能
	mov     cr0, eax
	jmp     .protectModeEntrance
.protectModeEntrance:
	nop
	
	jmp     pageafter	; 进入内核
