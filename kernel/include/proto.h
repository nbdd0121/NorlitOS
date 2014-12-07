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
     This file is the prototype header.
	 All C files in kernel should use the types defined by
		this file, in order to make use of symbols.
     The path of this source code is kernel/include/proto.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "type.h"
#include "const.h"
#include "asm.h"

/**** INITIALIZERs ****/

/* start.c */
INITIALIZER ASMLINKAGE void cstart();

/* interrupt.asm */
INITIALIZER ASMLINKAGE void init_8259A();

/* memory.c */
INITIALIZER FASTCALL void init_memory();

/* paging.c */
INITIALIZER FASTCALL void init_paging(u32);
INITIALIZER FASTCALL void init_paging_ap();

/* proc.c */
INITIALIZER FASTCALL void init_proc();
INITIALIZER FASTCALL void init_proc_ap();

/* timer.c */
INITIALIZER FASTCALL void init_timer();
INITIALIZER FASTCALL void init_timer_ap();
INITIALIZER FASTCALL void printStartupTime();

/* apic.c */
INITIALIZER FASTCALL void init_apic_ap();
INITIALIZER FASTCALL void init_apic(u32);
INITIALIZER FASTCALL void init_mp(u32 count);

/* acpi.c */
INITIALIZER FASTCALL void init_acpi();

/* keyboard.c */
INITIALIZER FASTCALL void init_keyboard();

/* mouse.c */
INITIALIZER FASTCALL void init_mouse();

/* video.c */
INITIALIZER FASTCALL void init_video();

/* windows.c */
INITIALIZER FASTCALL void init_windows();

/* harddisk.c */
INITIALIZER FASTCALL s32 init_harddisk();

/* filesystem.c */
INITIALIZER FASTCALL void init_fs();

/**** General Declarations ****/

/* int.c */
ASMLINKAGE void inner_exception_handler(u32, u32);

/* interrupt.asm */
ASMLINKAGE void _start();
ASMLINKAGE void unknown_irq(u32);

/* lib.c */
ASMLINKAGE void puts(const u8*);
ASMLINKAGE void* memcpy(void*, const void*, u32);
ASMLINKAGE void* memset(void*, u8, u32);
FASTCALL u32 memcmp(const void*, const void *, u32);
ASMLINKAGE u32 strlen(const u8*);
ASMLINKAGE u8* strcpy(u8*, const u8*);
FASTCALL u8 checksum(const void*, u32);
FASTCALL u8 BCD2BIN(u8 val);
FASTCALL u8 BIN2BCD(u8 val);
FASTCALL u64 do_divmod64(u64, u64, u64*);
FASTCALL u64 do_mod64(u64 num, u64 den);
FASTCALL u64 do_div64(u64 num, u64 den);
FASTCALL u64 mktime(u32, u8, u8, u8, u8, u8);
FASTCALL void gmtime(u64, Time*);
FASTCALL void delay(u32 millisec);
FASTCALL void readPort(u32, void*, u32);
FASTCALL void writePort(u32 port, void* buffer, u32 count);
FASTCALL u32 stringHash(u8 *str);
s32 fileOpen(u32 proc, u8* name);
u8* fileRead(u32 proc, u32 handle, u32 offset, u32 count);
u8* fileBuffer(u32 proc, u32 handle);
s32 fileWrite(u32 proc, u32 handle, u32 offset, u32 count);
#define BIT_TEST(x,bit) (((x)&(1<<(bit)))!=0)
#define BIT_SET(x,bit) ((x)|(1<<(bit)))
#define BIT_CLEAR(x,bit) ((x)&~(1<<(bit)))
#define BIT_SWITCH(x,bit) ((x)^(1<<(bit)))
#define assert(assertion) do{if(!(assertion)){io_clihlt();}}while(0)

/* format.c */
ASMLINKAGE u32 printf(const u8* fmt, ...);
ASMLINKAGE u32 vsprintf(u8* buf, const u8* fmt, u8* args);
ASMLINKAGE u32 sprintf(u8* buf, const u8* fmt, ...);
FASTCALL s32 atoi(u8* p);

/* memory.c */
FASTCALL void memory_block_free(u32 address, u8 bit);
FASTCALL void memory_free_nocheck(u32, u32);
FASTCALL void memory_free(u32, u32);
FASTCALL void* memory_alloc(u8);
FASTCALL void* malloc(u32);
FASTCALL void free(void*);

/* paging.c */
#define ALLOC_PAGE() memory_alloc(12)
#define FREE_PAGE(addr) memory_block_free((u32)(addr), 12);
#define ALLOC_PAGES(num) memory_alloc(12+(num))	//	Allocate 2^num
#define FREE_PAGES(addr, num) memory_block_free((u32)(addr), 12+(num))	//	Free 2^num
#define va2pa(x) ((u32)(x)-PAGE_OFFSET)
#define pa2va(x) ((u32)(x)+PAGE_OFFSET)
#define va2pde(x) ((u32)(x)>>22)
#define va2pte(x) (((u32)(x)&0x3FFFFF)>>12)
#define va2flat_pte(x) ((u32)(x)>>12)
#define rmfarptr2ptr(x) ((x).seg*0x10+(x).offset)

/* proc.c */
FASTCALL void schedule(u32 flag);
FASTCALL Process* createProcess(u32, u32, u8 *, u32);
FASTCALL void disposeProcess(Process*);
ASMLINKAGE void system_call(u32,u32,u32,u32);
FASTCALL void sendInterrupt(Process*);
#define NEW_THREAD(ip) ({u32 ret;asm volatile("int $0x30":"=a"(ret):\
	"d"(ip),"a"(SYSCALL_CREATE_THREAD));ret;})
#define DISPOSE_THREAD(thr) do{asm volatile("int $0x30"::"d"(thr),\
	"a"(SYSCALL_DISPOSE_THREAD));}while(0)
#define EXIT() do{asm volatile("int $0x30"::"d"(0),\
	"a"(SYSCALL_DISPOSE_THREAD));}while(0)
#define openPhysicalMemory(phy,lin,size) ({u32 ret;\
	asm volatile("int $0x30":"=a"(ret):\
	"d"(phy),"c"(lin),"b"(size),\
	"a"(SYSCALL_OPEN_PHYSICAL_MEMORY):"memory");ret;})
#define allocMemory(lin,size) ({u32 ret;\
	asm volatile("int $0x30":"=a"(ret):"c"(lin),"b"(size),\
	"a"(SYSCALL_ALLOC_MEMORY):"memory");ret;})
#define sendMessage(proc,lin) ({u32 ret;\
	asm volatile("int $0x30":"=a"(ret):"b"(0),"c"(lin),"d"(proc),\
	"a"(SYSCALL_SEND):"memory");ret;})
#define receiveMessage(lin) ({u32 ret;\
	asm volatile("int $0x30":"=a"(ret):"c"(lin),\
	"a"(SYSCALL_RECEIVE):"memory");ret;})
#define NICE(lin) do{asm volatile("int $0x30"::\
	"a"(SYSCALL_NICE));}while(0)

/* page.c */
FASTCALL u32 allocPhysicalPage();
FASTCALL void recyclePhysicalPage(u32);
FASTCALL u32 createProcessPDE(u32);
FASTCALL u32 allocThreadStack(Process*);
FASTCALL void recycleThreadStack(Thread*);
FASTCALL void recycleProcessPDE(u32);
ASMLINKAGE void pageFaultHandler(u32, u32);
FASTCALL s32 open_physical_memory(Process*, u32, u32, u32, u32);

/* proc.asm */
ASMLINKAGE void restart();
ASMLINKAGE void save();
ASMLINKAGE void spin_lock(u32*);
ASMLINKAGE void spin_unlock(u32*);
#define ENTER_CRITICAL(variable) {spin_lock(&(variable));MEMORY_BARRIER();{
#define LEAVE_CRITICAL(variable) }MEMORY_BARRIER();spin_unlock(&(variable));}
#define LEAVE_AND_RETURN(variable) do{MEMORY_BARRIER();spin_unlock(&(variable));return;}while(0)
#define DRIVER_ENTER_CRITICAL(variable) {MEMORY_BARRIER();io_cli();if(cpuPage.cpuid!=0)io_sti();spin_lock(&(variable));{
#define DRIVER_LEAVE_CRITICAL(variable) }spin_unlock(&(variable));io_sti();MEMORY_BARRIER();}

/* driver-rtc.c */
FASTCALL u8 CMOS_read(u32);
FASTCALL void CMOS_write(u32, u8);
FASTCALL void write_RTC(u64);
FASTCALL u64 read_RTC();

/* apic.c */
FASTCALL void registerIRQ(u8, IntHandler);
ASMLINKAGE void enableIRQ(u8);
ASMLINKAGE void disableIRQ(u8);

/* video.c */
ASMLINKAGE void putc(u8);
ASMLINKAGE void video_driver();

/* proclib.c */
FASTCALL s32 send(u32, u32);
FASTCALL s32 receive(u32 proc, u32 linear);
FASTCALL s32 sendReceive(u32 proc, u32 linear);
