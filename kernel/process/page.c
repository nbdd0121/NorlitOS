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
     This file is the part dealing with page management of process.
     The path of this source code is kernel/process/page.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "list.h"
#include "asm.h"
#include "type.h"
#include "global.h"

/*
 * The PDE and PTE provides OS 3 or 4 available bits to manage there
 * software. Norlit defines the value there (present pages) as
 * following:
 * 
 * 00: Normal Page, allocated by system
 * 
 */

FASTCALL u32 allocPhysicalPage(){
	u32 ret=va2pa(ALLOC_PAGE());
	//memset(pa2va(ret), 0, PAGE_SIZE);
	return ret;
}

FASTCALL void recyclePhysicalPage(u32 addr){
	FREE_PAGE(pa2va(addr));
	//freeMemoryCounter+=0x1000;
	//putc('~');
}

FASTCALL u32 createProcessPDE(u32 cr3Copy){
	u32* pde=ALLOC_PAGE();
	u32 offset=va2pde(PAGE_OFFSET);
	memset(pde, 0, offset*4);
	/* We first zeroify all unused area */
	memcpy(pde+offset,(u32*)pa2va(cr3Copy&~0xFFF)+offset, PAGE_SIZE-offset*4);
	/* Copy the kernel entries to the higher half */
	return va2pa(pde);
}

FASTCALL void recycleProcessPDE(u32 cr3){
	u32* pde=(u32*)pa2va(cr3&~0xFFF);
	u32 index;
	for(index=0;index<va2pde(STACK_TOP);index++){
		if(pde[index]!=0){
			u32* pte=(u32*)pa2va(pde[index]&~0xFFF);
			u32 pteIndex;
			for(pteIndex=0;pteIndex<1024;pteIndex++){
				if(pte[pteIndex]!=0&&!(pte[pteIndex]&PAGE_TYPE_PHY)){
					recyclePhysicalPage(pte[pteIndex]&~0xFFF);
				}
			}
			FREE_PAGE(pte);
		}
	}
	FREE_PAGE(pa2va(cr3));
}

FASTCALL void recycleThreadStack(Thread* thr){
	u32* pde=(u32*)pa2va(thr->process->cr3&~0xFFF);
	u32 pdeEntry=(va2pde(thr->stacktop-1)/2)*2+1;
	u32* pte=(u32*)pa2va(pde[pdeEntry]&~0xFFF);
	u32 num;
	for(num=0;num<1024;num++){
		if(pte[num]!=0){
			recyclePhysicalPage(pte[num]&~0xFFF);
		}
	}
	FREE_PAGE(pte);
	if(pde[pdeEntry-1]){
		pte=(u32*)pa2va(pde[pdeEntry-1]&~0xFFF);
		for(num=0;num<1024;num++){
			if(pte[num]!=0){
				recyclePhysicalPage(pte[num]&~0xFFF);
			}
		}
		FREE_PAGE(pte);
		pde[pdeEntry-1]=0;
	}
	pde[pdeEntry]=0;
}

FASTCALL u32 allocThreadStack(Process* proc){
	u32* pde=(u32*)pa2va(proc->cr3&~0xFFF);
	u32 pdeEntry=va2pde(STACK_TOP)-1;
	for(pdeEntry=va2pde(STACK_TOP)-1;pdeEntry>va2pde(STACK_BOTTOM);pdeEntry-=2){
		if(pde[pdeEntry]==0)break;
	}
	if(pdeEntry>va2pde(STACK_BOTTOM)){
		u32* stack=ALLOC_PAGE();
		memset(stack, 0, PAGE_SIZE-sizeof(u32));
		stack[1024-1]=allocPhysicalPage()|0b11111;
		pde[pdeEntry]=va2pa(stack)|0b11111;
		return (pdeEntry+1)*PAGE_SIZE*1024;
	}
	return 0;
}

ASMLINKAGE void pageFaultHandler(u32 vector, u32 errCode){
	Process* p=cpuPage.current_proc;
	Thread* t=cpuPage.current_thread;
	u32 pageAddr=asm_getRegister(cr2);
	if(pageAddr>(t->stacktop-0x800000)&&pageAddr<t->stacktop){
		if(pageAddr<(t->stacktop-0x800000+PAGE_SIZE)){
			printf("\fStack Overflow.");
		}else{
			/* Stack Expansion */
			//if(pageAddr<0xBFFD0000){printf("Internal Error %p",pageAddr);io_clihlt();}
			u32* pde=(u32*)pa2va(p->cr3&~0xFFF);
			u32 pdeEntry=va2pde(pageAddr);
			if(pde[pdeEntry]==0){
				u32* newpde=ALLOC_PAGE();
				memset(newpde, 0, PAGE_SIZE);
				pde[pdeEntry]=va2pa(newpde)|0b11111;
			}
			u32* pte=(u32*)pa2va(pde[pdeEntry]&~0xFFF);
			pte[va2pte(pageAddr)]=allocPhysicalPage()|0b11111;
			return;
		}
	}else{
		printf("\fA Page Fault was detected.");
	}
	schedule(2);
	printf(" [CPU%d]",cpuPage.cpuid);
	printf("Stack Top: 0x%p",t->stacktop);
	printf("\r\n\tLinear Address: 0x%p",pageAddr);
	printf("\r\n\tError Code: 0x%p",errCode);
	printf("\r\n"
		"\tSource: 0x%X : 0x%p\r\n"
		"\tEFlags: 0x%06X\r\n"
		"Process %s was killed.\r\n",
		t->regs.cs,	t->regs.ip,	t->regs.flags,	p->name);
		StackFrame* sf=&cpuPage.current_thread->regs;
	printf("\r\n"
		"EAX: 0x%p ECX: 0x%p\r\n"
		"EDX: 0x%p EBX: 0x%p\r\n"
		"ESP: 0x%p EBP: 0x%p\r\n"
		"ESI: 0x%p EDI: 0x%p\r\n"
		"EIP: 0x%p\r\n"
		"EFLAGS: 0x%06X\r\n",
		sf->ax, sf->cx, sf->dx, sf->bx, sf->sp, sf->bp, sf->si, sf->di,
		sf->ip, sf->flags);
	disposeProcess(p);
}

FASTCALL s32 open_physical_page(Process* proc, u32 phyaddr, u32 linearAddr, u32 flag){
	u32* cr3=(u32*)(pa2va(proc->cr3));
	u32* pde=&cr3[va2pde(linearAddr)];
	if(*pde==0){
		u32* newpde=ALLOC_PAGE();
		memset(newpde, 0, PAGE_SIZE);
		*pde=va2pa((u32)newpde)|0b11111;
		//printf("New PDE Created, Address %p\r\n",*pde);
	}
	u32* ptes=(u32*)(pa2va(*pde&~0xFFF));
	u32* pte=&ptes[va2pte(linearAddr)];
	if(*pte!=0){
		//printf("Page Alreay Existed.\r\n");
		//printf("CR3 Information: %p->%p->%p\r\n", linearAddr, *pde, *pte);
		return -1;
	}else{
		*pte=phyaddr|0b11111|flag;
		return 0;
		//printf("Page %p Mapped to %p.\r\n",linearAddr, phyaddr);
	}
}

FASTCALL s32 open_physical_memory(Process* proc, u32 phyaddr, u32 linearAddr, u32 pages, u32 flag){
	for(;pages;pages--,phyaddr+=PAGE_SIZE,linearAddr+=PAGE_SIZE){
		if(open_physical_page(proc,phyaddr,linearAddr,flag)==-1)return -1;
	}
	return 0;
}
