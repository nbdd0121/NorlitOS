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
#include "asm.h"
#include "type.h"
#include "global.h"

static u32 memoryLock=0;

FASTCALL void memory_block_free(u32 address, u8 bit){
	ENTER_CRITICAL(memoryLock);
	u32* ll=(u32*)address;
	u32** head=&memoryBlocks[bit-2];	// Find the correct list head
	u32 *entry=*head;
	u32 **prev=head;
	u32 sizep=1<<bit;
	freeMemoryCounter+=sizep;
	for(;entry!=EMPTY;prev=(u32**)entry,entry=*prev){	// Iterate the list
		if(entry>ll){
			if(!(address&sizep)&&(address+sizep==(u32)entry)){
				*prev=(u32*)(*entry);
				freeMemoryCounter-=sizep*2;
				spin_unlock(&memoryLock);
				memory_block_free(address, bit+1);
				return;
			}
			break;
		}else if((address&sizep)&&((u32)entry+sizep==address)){
			*prev=(u32*)(*entry);
			freeMemoryCounter-=sizep*2;
			spin_unlock(&memoryLock);
			memory_block_free((u32)entry, bit+1);
			return;
		}
	}
	*prev=ll;
	*ll=(u32)entry;
	LEAVE_CRITICAL(memoryLock);
}

FASTCALL void memory_free_nocheck(u32 address, u32 size){
	u8 bit;
	for(bit=2;bit<32;bit++){
		u32 sizep=1<<bit;
		if(size<sizep)break;
		if(address&sizep){
			memory_block_free(address,bit);
			address+=sizep;
			size-=sizep;
		}
		if(size&sizep){
			memory_block_free(address+size-sizep,bit);
			size-=sizep;
		}
	}
}

FASTCALL void memory_free(u32 address, u32 size){
	if(size<4)return;	/* A block will less than 4 bytes will waste memory */
	if(address&3){		/* An unaligned address */
		u32 afadd=(address&~3)+4;	/* Find the higher minimum aligned address */
		if((size=size+address-afadd)<4)return;	
					/* Find the new size, and make sure the new size was aligned */
		address=afadd;
	}
	if(size&3){			/* An unaligned size */
		size&=~3;		/* Just ignore the exceeded part */
	}
	memory_free_nocheck(address,size);
}

FASTCALL void* memory_alloc(u8 bit){
	u8 bit2=bit-2;	/* Starts at 4 */
	u32** head=&memoryBlocks[bit2];	/* Get the list head */
	u32* getit;
	ENTER_CRITICAL(memoryLock);
	getit=*head;	/* Get the first element */
	u32 fc=freeMemoryCounter;
	if(getit==EMPTY){	/* EMPTY element */
		do{
			if(bit2==27){
				printf(":-( Norlit OS ran out of memory.");
				io_clihlt();
			}
			head=&memoryBlocks[++bit2];	/* Seperate */
			getit=*head;
		}while(getit==EMPTY);
		*head=*((u32**)getit);	/* Detach */
		freeMemoryCounter-=(1<<(bit2+2));
		u32 extra=(1<<(bit2+2))-(1<<bit);	/* Calcuate Extra */
		spin_unlock(&memoryLock);
		memory_free_nocheck(((u32)getit)+(1<<bit),extra);	/* Return */
		return (void*)getit;
	}else{
		freeMemoryCounter-=(1<<bit);
		*head=*((u32**)getit);	/* Detach */
	}
	LEAVE_CRITICAL(memoryLock);
	return (void*)getit;
}

FASTCALL void* malloc(u32 size){
	u8 bit;
	size=((size-1+sizeof(u32))&~3)+4;	/* Find out the aligned size */
	u32 sized=(size-1)>>2;
	for(bit=2;sized;bit++,sized>>=1);	/* Get the greater minimum power of two */
	if(bit>29){
		puts(":-( Norlit OS ran out of memory.!!");
		io_clihlt();
	}
	void* getit=memory_alloc(bit);	/* Allocate memory */
	*((u32*)getit)=1<<bit;	/* Store the allocated size */
	return getit+sizeof(u32);
}

FASTCALL void free(void* addr){
	u32* ad=(u32*)(addr-sizeof(u32));	/* Find out the size-storing field */
	memory_free_nocheck((u32)ad, *ad);	/* Free the block */
}

INITIALIZER FASTCALL void init_memory(){
	BootParam* bp=(BootParam*)BOOT_PARAM_POS;
	u32 bplen=bp->len;
	ARDSItem* ai=bp->items;
	u64 max=0;
	for(;bplen>0;bplen--,ai++){
		printf("[  0.000] MEM: Block 0x%p-0x%p ", (u32)ai->base, (u32)(ai->base+ai->limit-1));
		switch(ai->type){
		case 5:puts("Unusable\r\n");continue;
		case 6:puts("Disabled\r\n");continue;
		case 1:{
			puts("Avaliable\r\n");
			if(ai->base<LOADER_MAPPED){
				u32 base=ai->base,limit=ai->limit;
				if(base<KERNEL_OFFSET-PAGE_OFFSET+KERNEL_SIZE){
					limit+=base;
					if(limit<=KERNEL_OFFSET-PAGE_OFFSET+KERNEL_SIZE)break;
					base=KERNEL_OFFSET-PAGE_OFFSET+KERNEL_SIZE;
					limit-=base;
				}
				if(base+limit>LOADER_MAPPED){
					limit=LOADER_MAPPED-base;
				}
				memory_free_nocheck(PAGE_OFFSET+base,limit);
			}
			break;
		}
		case 3:puts("ACPI\r\n");break;
		case 4:puts("NVS\r\n");break;
		default:puts("Reserved\r\n");continue;
		}
		max=ai->base+ai->limit;
	}
	printf("[  0.000] MEM: Size: %dMB\r\n",max/0x100000);
	
	assert(max>=0x1000000);
	/* Panic if the memory size is less than 16MB */
	
	if(max>0xFFFFFFFF){
		puts("Your memory is over 4GB. Please Turn on PAE if you want to use them.");
	}
	init_paging(max);
	for(bplen=bp->len,ai=bp->items;bplen>0;bplen--,ai++){
		if(ai->type==1&&ai->base<MAX_MAPPING){
			u32 base=ai->base,limit=ai->limit;
			
			if(base<LOADER_MAPPED){
				limit+=base;
				if(limit<=LOADER_MAPPED)continue;
				base=LOADER_MAPPED;
				limit-=base;
				//continue;
			}
			if(base+limit>MAX_MAPPING){
				limit=MAX_MAPPING-base;
			}
			
			memory_free_nocheck(PAGE_OFFSET+base,limit);
		}
	}
}
