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
     This file is the part dealing with memory allocating.
     The path of this source code is kernel/memory/memory.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "global.h"

INITIALIZER FASTCALL void init_paging_ap(){
	asm_setCR3(va2pa(kernelPDE));
	u32* kernel_page=ALLOC_PAGE();
	memcpy(kernel_page, flatPTEs, 0x1000);
	int i=0;
	kernel_page[va2flat_pte(KERNEL_STRUCTURE)]=(u32)ALLOC_PAGE()-PAGE_OFFSET+0b11111;
	kernelPDE[va2pde(PAGE_OFFSET)]=va2pa(kernel_page)+0b11111;
	asm_setCR3(va2pa(kernelPDE));
}

INITIALIZER FASTCALL void init_paging(u32 memsize){
	u32 index;
	u32 pages=va2flat_pte(memsize);
	flatPTEs=memory_alloc(22);// Allocate at most 4MB page table entrys
	u32 allocpages=(pages+1024-1)&~(1024-1);
	{
		memory_free_nocheck(((u32)flatPTEs)+allocpages*4,(1<<22)-allocpages*4);
			// Release extra memory
		u32* pteptr=flatPTEs;
		for(index=0;index<pages;index++,pteptr++){
			*pteptr=index*0x1000+0b11111;
		}
		for(;index<allocpages;index++,pteptr++){
			*pteptr=index*0x1000;	// Page not present
		}
	}/* This block initializes the PTEs for flat model. */
	
	u32* kernel_page=ALLOC_PAGE();
	memcpy(kernel_page, flatPTEs, 0x1000);
	kernel_page[va2flat_pte(KERNEL_STRUCTURE)]=(u32)ALLOC_PAGE()-PAGE_OFFSET+0b11111;
	
	kernelPDE=ALLOC_PAGE();
	{
		u32* pdeptr=kernelPDE;
		u32 maxpdeentry=allocpages/1024;
		maxpdeentry=maxpdeentry>(va2pde(MAX_MAPPING))?(va2pde(MAX_MAPPING)):maxpdeentry;
		for(index=0;index<va2pde(PAGE_OFFSET);index++,pdeptr++){
			*pdeptr=0;	// Page not present
		}
		*pdeptr=va2pa(kernel_page)+0b11111;
		pdeptr++;
		for(index=1;index<maxpdeentry;index++,pdeptr++){
			*pdeptr=va2pa(flatPTEs)+index*0x1000+0b11111;
		}
		for(;index<1024-(va2pde(PAGE_OFFSET));index++,pdeptr++){
			*pdeptr=0;	// Page not present
		}
	}/* This block initializes the PDEs used for kernel processes. */
	asm_setCR3(va2pa(kernelPDE));// Set CR3
}

