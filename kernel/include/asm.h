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
     This file is header to provide I/O operations.
     The path of this source code is kernel/include/asm.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once
 
#define io_cli() do{asm volatile("cli");}while(0)
#define io_sti() do{asm volatile("sti");}while(0)
#define io_hlt() do{asm volatile("hlt");}while(0)
#define io_clihlt() do{io_cli();io_hlt();}while(0)

static INLINE void io_out8(u16 port, u8 value){
	asm volatile("outb %1,%0"::"Nd"(port),"a"(value));
}
static INLINE u8 io_in8(u16 port){
	u8 inv;
	asm volatile("inb %1,%%al":"=a"(inv):"Nd"(port));
	return inv;
}

#define asm_cpuid(id, reg) ({u_addr ret;asm volatile("cpuid":"=" #reg (ret):"a"(id));ret;})

static INLINE u64 asm_rdmsr(u32 reg){
	u64 val;
	asm volatile("rdmsr":"=A"(val):"c"(reg));
	return val;
}
static INLINE u64 asm_wrmsr(u32 reg, u64 val){
	asm volatile("wrmsr"::"A"(val),"c"(reg));
}

#define asm_mfence() do{asm volatile("mfence":::"memory");}while(0)
#define asm_sfence() do{asm volatile("sfence":::"memory");}while(0)
#define asm_lfence() do{asm volatile("lfence":::"memory");}while(0)

#define asm_pause(x) do{asm volatile("pause");}while(0)
#define asm_nop(x) do{asm volatile("nop");}while(0)

#define asm_setCR3(x) do{asm volatile("movl %0,%%cr3"::"a"((x)|0b11000));}while(0)
#define asm_getRegister(reg) ({u32 val;asm volatile("movl %%" #reg ",%0":"=a"(val));val;})







