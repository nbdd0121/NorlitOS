/********************************************************************

   Copyright 2012-2013 by Gary Guo - All Rights Reserved

   * All source code or binary file can only be used as personal
     study or research, but can not be used for business.
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
     
     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
     LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ********************************************************************
     This file is the interrupt dealing part.
     The path of this source code is kernel/int.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "type.h"
#include "global.h"
#include "list.h"
#include "const.h"
#include "apic.h"

ASMLINKAGE void inner_exception_handler(u32 vec_no, u32 err_code){
	Process* p=cpuPage.current_proc;
	Thread* t=cpuPage.current_thread;
	schedule(2);
	static char * err_msg[] = {
		"#DE Divide Error",
		"#DB Debug Error",
		"    NMI Interrupt",
		"#BP Breakpoint",
		"#OF Overflow",
		
		"#BR Bound Range Exceeded",
		"#UD Invalid Opcode (Undefined Opcode)",
		"#NM Device Not Available (No Math Coprocessor)",
		"#DF Double Fault",
		"    Coprocessor Segment Overrun (reserved)",
		
		"#TS Invalid TSS",
		"#NP Segment Not Present",
		"#SS Stack-Segment Fault",
		"#GP General Protection",
		"#PF Page Fault",
		
		"    Intel Reserved",
		"#MF x87 FPU Floating-Point Error (Math Fault)",
		"#AC Alignment Check",
		"#MC Machine Check",
		"#XF SIMD Floating-Point Exception",
		
		"#VE Virtualization Exception",
		"    Hardware IRQ",
		"    Unexpected System Call"
	};
	if(vec_no==-1)vec_no=15;
	printf("%s [CPU%d]",err_msg[vec_no],cpuPage.cpuid);
	if(err_code!=-1){
		printf("\r\n     Error Code: 0x%p",err_code);
	}
	printf("\r\n"
		"     Source: 0x%X : 0x%p\r\n"
		"     EFlags: 0x%06X\r\n"
		"Process %s was killed.\r\n",
		t->regs.cs,	t->regs.ip,	t->regs.flags,	p->name);
	disposeProcess(p);
}

ASMLINKAGE void breakpoint_disp(){
	StackFrame* sf=&cpuPage.current_thread->regs;
	printf("\r\n"
		"#BP Breakpoint\r\n"
		"EAX: 0x%p ECX: 0x%p\r\n"
		"EDX: 0x%p EBX: 0x%p\r\n"
		"ESP: 0x%p EBP: 0x%p\r\n"
		"ESI: 0x%p EDI: 0x%p\r\n"
		"EIP: 0x%p\r\n"
		"EFLAGS: 0x%06X\r\n",
		sf->ax, sf->cx, sf->dx, sf->bx, sf->sp, sf->bp, sf->si, sf->di,
		sf->ip, sf->flags);
}

ASMLINKAGE void unknown_irq(u32 irq){
	inner_exception_handler(21,irq);
}

INITIALIZER ASMLINKAGE void init_8259A(){
	io_out8(INT_MASTER_CTL, INT_8259A_M_ICW1);
	io_out8(INT_MASTER_CTLMASK, INT_8259A_M_ICW2);
	io_out8(INT_MASTER_CTLMASK, INT_8259A_M_ICW3);
	io_out8(INT_MASTER_CTLMASK, INT_8259A_M_ICW4);
	io_out8(INT_SLAVE_CTL, INT_8259A_S_ICW1);
	io_out8(INT_SLAVE_CTLMASK, INT_8259A_S_ICW2);
	io_out8(INT_SLAVE_CTLMASK, INT_8259A_S_ICW3);
	io_out8(INT_SLAVE_CTLMASK, INT_8259A_S_ICW4);
	io_out8(INT_MASTER_CTLMASK, 0b11111111);
	io_out8(INT_SLAVE_CTLMASK, 0b11111111);
}
