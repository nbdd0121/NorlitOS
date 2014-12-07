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
     This file is the part dealing with ACPI.
     The path of this source code is kernel/driver/acpi.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#define USE_ACPI_TYPES 1

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "type.h"
#include "global.h"

/** 
* The acpi_check_rsdp method of the ACPI Driver. 
* This method is called when the system trys to
   * get check if a RSDP is valid. 
* @param rsdp
*  the RSDP table
* @return u32
*  1 if valid, 0 if invalid
*/
INITIALIZER static u32 acpi_check_rsdp(struct ACPI_RSDP* rsdp){
	/* Refers to the page 112 of the ACPI Specificaton 4.0a
	 * We check if RSDP's checksum is correct */
	if(checksum(rsdp, 20)!=0)return 0;
	/* We check if XSDT existed */
	if(rsdp->revision!=0){
		if(checksum(rsdp, rsdp->length)!=0)return 0;
	}else{
		/* If XSDT is not existed, we turn to use DSDT */
		rsdp->length=20;
	}
	return 1; 
}

#define TEMP_MAP_POSITION 0x40000000
INITIALIZER static u32 returnVirtualAddress(u32 phyaddr){
	if(!BIT_TEST(kernelPDE[va2pde(TEMP_MAP_POSITION)],0)){
		u32* pg=ALLOC_PAGE();
		memset(pg, 0, PAGE_SIZE);
		kernelPDE[va2pde(TEMP_MAP_POSITION)]=va2pa(pg)+0b11011;
	}
	u32* pgo=(u32*)(pa2va(kernelPDE[va2pde(TEMP_MAP_POSITION)])&~0xFFF);
	u32* pg=pgo;
	while(BIT_TEST(*pg,0))pg++;
	*pg=(phyaddr&~0xFFF)+0b11011;
	return TEMP_MAP_POSITION+((u32)pg-(u32)pgo)*1024+(phyaddr&0xFFF);
}

INITIALIZER static void cleanUpTempMaps(){
	if(BIT_TEST(kernelPDE[va2pde(TEMP_MAP_POSITION)],0)){
		FREE_PAGE(pa2va(kernelPDE[va2pde(TEMP_MAP_POSITION)])&~0xFFF);
		kernelPDE[va2pde(TEMP_MAP_POSITION)]=0;
	}
}

/** 
* The acpi_apic method of the ACPI Driver. 
* This method is called when the system trys to
   * get APIC information for MADT table. 
* @param madt
*  the MADT table
*/
INITIALIZER static void acpi_apic(struct ACPI_MADT* madt){
	u32 length=madt->header.length-offsetof(struct ACPI_MADT,apic_str);
	u8 pros=0;
	
	u32 io_apic_base=0;
	
	struct ACPI_APICHeader* header=madt->apic_str;
	while(length){
		switch(header->type){
		case 0:{
			pros++;
			break;
		}
		case 1:{
			struct ACPI_IO_APIC* apic=(struct ACPI_IO_APIC*)header;
			if(io_apic_base!=0){
				puts(":-( Mulitiple I/O APIC Not Supported.");
				continue;
			}
			io_apic_base=apic->io_apic_addr;
			break;
		}
		case 2:{
			struct ACPIIntOverride* irq=(struct ACPIIntOverride*)header;
			irqOverride[irq->source]=irq->interrupt;
			break;
		}
		}
		length-=header->length;
		header=(struct ACPI_APICHeader*)((u32)header+header->length);
	}
	
	assert(io_apic_base!=0);
	/* Panic if no APIC support */
	
	init_apic(io_apic_base);
	
	if(pros>1)
		init_mp(pros);
}

/** 
* The acpi_find_rsdp method of the ACPI Driver. 
* This method is called when the system trys to 
   * find the RSDP Table. 
* @return struct ACPI_RSDP* 
*  the RSDP Table found
*/
INITIALIZER static struct ACPI_RSDP* acpi_find_rsdp(){
	/* Refers to the page 111 of the ACPI Specificaton 4.0a */
	u8* sig;
	/* ACPI's RSDP can be located in the BIOS read-only
	 *  memory space between 0E0000h and 0FFFFFh. */
	for(sig=(u8*)0xC00E0000;sig<(u8*)0xC00FFFFF;sig+=16)
		/* We check if we found a RSDP */
		if(memcmp(sig,"RSD PTR ",8)==0&&acpi_check_rsdp((struct ACPI_RSDP*)sig))
			return (struct ACPI_RSDP*)sig;
	/* * We first found out the EBDA(Extended BIOS Data Area)'s Address */
	u32 ebda=*((u16*)(0x40E + PAGE_OFFSET))*0x10+PAGE_OFFSET;	
	/* ACPI's RSDP can also be located in,
	 * the first 1 KB of the EBDA. */
	for(sig=(u8*)ebda;sig<(u8*)(ebda+0x3FF);sig+=16)
		/* We check if the signature is RSD PTR */
		if(memcmp(sig,"RSD PTR ",8)==0&&acpi_check_rsdp((struct ACPI_RSDP*)sig))
			return (struct ACPI_RSDP*)sig;
	return NULL;
}

/** 
* The init_acpi method of the ACPI Driver. 
* This method is called when the system trys to
	* initialize ACPI. 
*/
INITIALIZER FASTCALL void init_acpi(){
	struct ACPI_RSDP* rsdp=acpi_find_rsdp();
	
	assert(rsdp!=NULL);
	/* Panic if no ACPI support */
	
	struct ACPI_RSDT* rsdt=(struct ACPI_RSDT*)returnVirtualAddress(rsdp->rsdt);
	u32 max_entries=(rsdt->header.length-offsetof(struct ACPI_RSDT, entry))/4;
	u32 index=0;
	puts("[  0.000] ACPI: RSDT Found.\r\n");
	for(index=0;index<max_entries;index++){
		struct ACPIHeader *entry=(struct ACPIHeader*)returnVirtualAddress(rsdt->entry[index]);
		if(entry->signature==0x43495041){	//MADT
			acpi_apic((struct ACPI_MADT*)entry);
			break;
		}
	}
	cleanUpTempMaps();
}
