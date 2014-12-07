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
;     This file is the process manager in the kernel.
;     The path of this source code is kernel/process/proc.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

[BITS 32]

[section .text]	; 代码在此

%include "const.inc"

global save
global restart
global spin_lock
global spin_unlock

restart:
	mov		edx, [kernel_str]
	mov		edx, [edx]
	or		edx, 0b11000
	
	mov		ebp, [kernel_str+4]
	lea		esp, [ebp+0x1000-(4+8+1+5)*4]
	; Note: 0x1000 should be 0x1000*2^PROCESS_STRUCT_SIZE
	;	4+8+1+5 = sregs + GPRs + retaddr + iretd's args
	;	equals to mov esp, &current_thread->regs

	mov		eax, cr3
	cmp		edx, eax
	jne		.chgcr3
	jmp		.chgcr3
.nochg:
	pop		gs
	pop		fs
	pop		es
	pop		ds
	popad
	add		esp, 4
	iretd
.chgcr3:
	mov		cr3, edx
	jmp		.nochg

save:
	pushad
	push	ds
	push	es
	push	fs
	push	gs
	mov		si, ss
	mov		ds, si
	mov		es, si
	push	restart
	jmp		[esp+(4+8+1)*4]
		; 4 Seg Regs, 8 GPRs, 1 `restart`
		
spin_lock:
	mov		edx, [esp+4]
.spin:
	mov		eax, 1
	xchg	eax, [edx]
	or		eax, eax
    jz		.ret
	pause
	jmp		.spin
.ret:
    ret

spin_unlock:
	mov		edx, [esp+4]
	mov		dword[edx], 0
	ret
	
