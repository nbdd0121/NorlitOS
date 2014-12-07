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
     This file is the entry point of the kernel.
     The path of this source code is kernel/start.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "const.h"
#include "proto.h"
#include "type.h"
#include "apic.h"

#ifdef BIT64
#error Norlit OS cannot be compiled in 64 bit.
#endif

/** 
 * cstart initializes GDT, IDT and other components of the system.
 */ 
INITIALIZER ASMLINKAGE void cstart(){
	/* Use bpflag to do different action on different system */
	u8 bpFlag=BIT_TEST(asm_rdmsr(IA32_APIC_BASE),8);
	
	/* Get paging and memory ready */
	if(bpFlag){
		init_8259A();
		init_memory();
	}else{
		init_paging_ap();
	}

	/* Convert GDTTable into a u64 array.
	 * We will just set all entries to a calculated descriptor */
	u64 *gdt=(u64*)cpuPage.GDTTable;
	
	gdt[0]=0x0000000000000000;	/* Dummy Descriptor */
	gdt[1]=0x00CF9B000000FFFF;	/* Flat System Code Descriptor */
	gdt[2]=0x00CF92000000FFFF;	/* Flat System Data Descriptor */
	gdt[3]=0x00CFFA000000FFFF;	/* Flat User Code Descriptor */
	gdt[4]=0x00CFF2000000FFFF;	/* Flat User Data Descriptor */
	gdt[5]=0x0000890000000068;	/* TSS Descriptor */
	
	/* Initializes non-predefined part of TSS */
	u32 adr=(u32)&cpuPage.tss;
	cpuPage.GDTTable[5].baseLow=adr;
	cpuPage.GDTTable[5].baseMiddle=adr>>16;
	cpuPage.GDTTable[5].baseHigh=adr>>24;

	/* Clear TSS and set SS0 */
	memset(&cpuPage.tss, 0, sizeof(TSS));
	cpuPage.tss.ss0=sizeof(u64)*2;
	
	/* Initialize a GDT Pointer */
	struct{
		u16 GDTLimit;
		GDTItem* GDTBase;
	} GDTPointer;
	GDTPointer.GDTBase=cpuPage.GDTTable;
	GDTPointer.GDTLimit=sizeof(u64)*GDT_LENGTH;
	
	/* Load GDT and TSS into GDT Register and Task Register */
	asm volatile("lgdt (%0)":: "r" (&GDTPointer):"memory");
	asm volatile("ltr %%ax":: "a"(sizeof(u64)*5):"memory");
	
	
	
	/* Initialize the interrupt handlers */
	u32 index;
	for(index=0;index<0x20;index++){
		/* The first 0x20 interrupts are internal exceptions */
		cpuPage.handlerTable[index]=inner_exception_handler;
	}
	for(index=0;index<IDT_LENGTH-0x20;index++){
		/* The other interrupts are hardware IRQs or system calls */
		cpuPage.handlerTable2[index]=unknown_irq;
	}
	
	/* Don't forget to initialize system call */
	cpuPage.handlerTable[0x30]=(ExceptionHandler)system_call;
	
	cpuPage.kernelStack=ALLOC_PAGE()+0xF00;
	
	/* BP need to initialize video and other devices */
	if(bpFlag){	
		init_acpi();
		init_proc();
		init_video();		/* PID 1 */
		if(init_harddisk()){/* PID 2 */
			init_windows();	/* PID 3 */
			init_fs();		/* PID 4 */
			init_keyboard();
			init_mouse();
		}
		/* If no harddisk is avaliable, system will panic. */
		init_timer();
	}else{
		init_apic_ap();
		init_proc_ap();
		init_timer_ap();

		restart();
	}
}

