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
     This file is the part dealing with APIC.
     The path of this source code is kernel/driver/apic.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "type.h"
#include "global.h"
#include "apic.h"

INITIALIZER FASTCALL void init_apic_ap(){
	asm_wrmsr(IA32_APIC_BASE, BIT_SET(BIT_SET(APIC_REG_ADDR,8),11));
	/* Enable APIC if not */

	apic_write(APIC_SVR,BIT_SET(apic_read(APIC_SVR),8));
	apic_write(APIC_LVT_ERR,0x40);
	
	u8 apic_id=apic_read(APIC_ID_REG)>>24;
	
	cpuPage.cpuid=apic_id;
	
	printf("[  0.000] APIC: AP%d Initialized.\r\n",apic_id);
}

INITIALIZER FASTCALL void init_apic(u32 io_apic_addr){
	bootParam.lock=1;
	
	/* Initialize the page table for APICs */
	u32* apic_ptes=ALLOC_PAGE();
	memset(apic_ptes, 0, PAGE_SIZE);

	/* Map I/O APIC */
	apic_ptes[va2pte(IO_APIC_ADDR)]=io_apic_addr+0b11011;
	
	asm_wrmsr(IA32_APIC_BASE, BIT_SET(BIT_SET(APIC_REG_ADDR,8),11));
	/* Enable APIC if not */
	
	apic_ptes[va2pte(APIC_REG_ADDR)]=APIC_REG_ADDR+0b11011;
	/* Map Local APIC */

	/* Map the page table */
	kernelPDE[va2pde(APIC_REG_ADDR)]=va2pa(apic_ptes)+0b11011;
	asm_setCR3(va2pa(kernelPDE));

	u8 apic_id=apic_read(APIC_ID_REG)>>24;
	cpuPage.cpuid=apic_id;
	
	u32 id;
	for(id=0;id<16;id++){
		io_apic_write(IO_APIC_RED(id)+1,apic_id<<(56-32));
		io_apic_write(IO_APIC_RED(id),0x10000+0x20+id);
	}
	for(;id<24;id++){
		io_apic_write(IO_APIC_RED(id)+1,apic_id<<(56-32));
		io_apic_write(IO_APIC_RED(id),0x10000);
	}
	for(id=0;id<16;id++){
		if(irqOverride[id]!=id)
			io_apic_write(IO_APIC_RED(irqOverride[id]),0x10000+0x20+id);
	}
	
	apic_write(APIC_SVR,BIT_SET(apic_read(APIC_SVR),8));
	apic_write(APIC_LVT_ERR,0x40);

	/* Display Info */
	printf("[  0.000] APIC: Local APIC ID: %d\r\n", apic_id);
	printf("[  0.000] APIC: I / O APIC ID: %d\r\n", io_apic_read(IO_APIC_ID)>>24);
	
}

INITIALIZER FASTCALL void init_mp(u32 count){
	#if !NO_SMP
	printf("[  0.000] APIC: Count of Processor: %d\r\n",count);
	u32 vector=SMP_LOADER_OFFSET/0x1000;
	apic_write(APIC_ICR_LOW,0xC4500);
	/* Init */
	volatile int i;
	for(i=0;i<0x10000;i++);
	/* Delay for some while */
	apic_write(APIC_ICR_LOW,0xC4600+vector);
	/* SIPI */
	#endif
}

FASTCALL void registerIRQ(u8 irq, IntHandler inth){
	cpuPage.handlerTable2[irq]=inth;
}

ASMLINKAGE void enableIRQ(u8 irq){
	u32 adr=IO_APIC_RED(irqOverride[irq]);
	io_apic_write(adr,BIT_CLEAR(io_apic_read(adr),16));
}

ASMLINKAGE void disableIRQ(u8 irq){
	u32 adr=IO_APIC_RED(irqOverride[irq]);
	io_apic_write(adr,BIT_SET(io_apic_read(adr),16));
}

