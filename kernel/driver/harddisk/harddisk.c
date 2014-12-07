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
     This file is the part dealing with hard disk.
     The path of this source code is kernel/driver/harddisk/harddisk.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"
#include "apic.h"
#include "harddisk.h"

INITIALIZER FASTCALL s32 init_harddisk(){
	diskAmount=*(u8*)(PAGE_OFFSET+0x475);
	if(!diskAmount){
		puts("[  0.000] HD: No Harddisks Detected.\r\n");
		return 0;
	}else{
		printf("[  0.000] HD: %d Harddisk(s) Detected.\r\n",diskAmount);
		registerIRQ(14, irq13_handler);
		enableIRQ(14);
		harddisk_proc=createProcess((u32)harddisk_bottom, 0, "Harddisk",
				DEVICE_DRIVER_PRIORITY);
		return 1;
	}
}

#define MSG_ADDR 0
#define INT_MSG_ADDR 0x1000
#define DATA_BUFFER 0x10000000
#define DATA_LOCATION 0x20000000
static void readSectors(u32 lba, u32 count, u8* data);
static void writeSectors(u32 lba, u32 count, u8* data);
static u32 readPartition(HardDisk* hd);

static ASMLINKAGE void harddisk_bottom(){
	allocMemory(DATA_BUFFER, 1);
	allocMemory(DATA_LOCATION, 1);
	u8* buf=(u8*)DATA_BUFFER;
	HardDisk* hd=(HardDisk*)DATA_LOCATION;
	hd->buffer=buf;
	identifyHD(hd);
	readPartition(hd);
	FileMessage* msg=(FileMessage*)MSG_ADDR;
	while(1){
		s32 proc=receiveMessage(MSG_ADDR);
		if(msg->signature!=COMMAND_HD)continue;
		switch(msg->command){
			case HD_OPEN:{
				if(msg->handle==48||msg->handle==1148601){/* 0 or $MBR */
					msg->handle=0;
					printStartupTime();
					printf(" HD: Process %x opens //Device/HD0/%s\r\n",proc,msg->data);
					memcpy(msg->data, hd, sizeof(HardDisk)>2048?2048:sizeof(HardDisk));
					sendMessage(proc, MSG_ADDR);
				}else{
					if(!(msg->handle=atoi(msg->data))){
						printStartupTime();
						printf(" HD: Cannot open //Device/HD0/%s\r\n",msg->data);
						msg->command=HD_FAIL;
						sendMessage(proc, MSG_ADDR);
					}else{
						printStartupTime();
						printf(" HD: Process %x opens //Device/HD0/%s\r\n",
								proc, msg->data);
						sendMessage(proc, MSG_ADDR);
					}
				}
				break;
			}
			case HD_READ:case HD_WRITE:{
				u32 lba=msg->offset+(msg->handle==0?0:hd->part[msg->handle-1].start);
				if(msg->command==HD_READ)
					readSectors(lba, msg->count, msg->data);
				else
					writeSectors(lba, msg->count, msg->data);
				sendMessage(proc, MSG_ADDR);
				break;
			}
		}
	}
}

static void readSectors(u32 lba, u32 count, u8* data){
	HDCommand cmd;
	cmd.command=ATA_READ;
	cmd.count=count;
	cmd.device=MASTER|LBA|((lba>>24)&0xF);
	cmd.lbaLow=lba&0xFF;
	cmd.lbaMid=(lba>>8)&0xFF;
	cmd.lbaHigh=(lba>>16)&0xFF;
	sendCommand(REG_BASE_PRIMARY, &cmd);
	while(count--){
		receive(-1, INT_MSG_ADDR);
		readPort(REG_BASE_PRIMARY+REG_OFFSET_DATA, data, SECTOR_SIZE);
		data+=SECTOR_SIZE;
	}
}

static void writeSectors(u32 lba, u32 count, u8* data){
	HDCommand cmd;
	cmd.command=ATA_WRITE;
	cmd.count=count;
	cmd.device=MASTER|LBA|((lba>>24)&0xF);
	cmd.lbaLow=lba&0xFF;
	cmd.lbaMid=(lba>>8)&0xFF;
	cmd.lbaHigh=(lba>>16)&0xFF;
	sendCommand(REG_BASE_PRIMARY, &cmd);
	while(count--){
		while(!BIT_TEST(io_in8(REG_BASE_PRIMARY+REG_OFFSET_STATUS),3))NICE();
		writePort(REG_BASE_PRIMARY+REG_OFFSET_DATA, data, SECTOR_SIZE);
		receive(-1, INT_MSG_ADDR);
		data+=SECTOR_SIZE;
	}
}

static u32 readPartition(HardDisk* hd){
	u32 i, extSec=0;
	u8* data=hd->buffer;
	MBRTable* mbr=(MBRTable*)data;
	readSectors(0x0, 1, data);
	if(mbr->endCheck!=MBR_CHECK){
		printStartupTime();
		puts(" HD: MBR Broken. \r\n");
		return 0;
	}
	if(mbr->entry[0].fileSystem!=MAGIC_GPT){
		for(i=0;i<4;i++){
			MBREntry *me=&mbr->entry[i];
			Partition *pt=&hd->part[i];
			pt->size=me->size;
			pt->start=me->location;
			pt->type=me->fileSystem|(me->active!=0)*0x80000000;
			if(me->fileSystem==MAGIC_EXTENDED){
				extSec=pt->start;
			}
		}
		u32 id=4;
		u32 extBase=extSec;
		while(extSec){
			if(id<MAX_PARTITIONS){
				Partition *npt=&hd->part[id++];
				readSectors(extSec, 1, data);
				if(mbr->endCheck!=MBR_CHECK){
					printStartupTime();
					printf(" HD: EBR %d Broken\r\n",extSec);
					break;
				}
				u32 i=0;
				npt->start=mbr->entry[0].location+extSec;
				npt->size=mbr->entry[0].size;
				npt->type=mbr->entry[0].fileSystem|
						(mbr->entry[0].active!=0)*0x80000000;
				if(mbr->entry[1].fileSystem){
					extSec=extBase+mbr->entry[1].location;
				}else{
					break;
				}
			}else{
				printStartupTime();
				printf(" HD: No More Than %d Partitions\r\n",MAX_PARTITIONS);
				break;
			}
		}
		hd->partCount=id;
	}else{
		hd->partCount=0;
		printStartupTime();
		printf(" HD: GPT Partitions Are Not Supported\r\n",MAX_PARTITIONS);
	}
	for(i=0;i<hd->partCount;i++){
		Partition *pt=&hd->part[i];
		if(!pt->type)continue;
		printStartupTime();
		printf(" HD: Partition %d: ", i+1);
		if(BIT_TEST(pt->type,31))putc('*');else putc(' ');
		u8* fs;
		switch(BIT_CLEAR(pt->type,31)){
			case MAGIC_EXTENDED:fs="Extended";break;
			case 0x07:fs="NTFS";break;
			case 0x82:fs="Swap";break;
			case 0x83:fs="Linux";break;
			case MAGIC_NORLIT:fs="Norlit";break;
			default:printf(" %02X",BIT_CLEAR(pt->type,31));fs="Unknown";
		}
		printf(" %-8s, %d-%d",fs,pt->start,pt->start+pt->size-1);
		puts("\r\n");
	}
}

static u32 identifyHD(HardDisk* hd){
	HDCommand cmd;
	u8* data=hd->buffer;
	cmd.command=ATA_IDENTIFY;
	cmd.device=MASTER;
	sendCommand(REG_BASE_PRIMARY, &cmd);
	receive(-1,INT_MSG_ADDR);
	readPort(REG_BASE_PRIMARY+REG_OFFSET_DATA, data, SECTOR_SIZE);
	printStartupTime();
	puts(" HD: Serial Number: ");
	printIdentifyStr(data+20, 20);
	puts("\r\n");
	printStartupTime();
	puts(" HD: Model Number: ");
	printIdentifyStr(data+54, 40);
	puts("\r\n");
	if(!BIT_TEST(*(u16*)&data[98],9)){
		printStartupTime();
		printf(" HD: Failed: No LBA Support.\r\n");
		return 0;
	}

	u32 lba48=BIT_TEST(*(u16*)&data[166],10);
	if(lba48){
		printStartupTime();
		printf(" HD: LBA48 Supported.\r\n");
	}

	hd->size=*(u32*)&data[120];
	printStartupTime();
	printf(" HD: Size: %dMB\r\n", (hd->size/2-1)/1024+1);
	printStartupTime();
	puts(" HD: Initialized.\r\n");
	return 1;
}

static void printIdentifyStr(u8* str, u32 length){
	u8 buf[64];
	u8* ptr=buf;
	u32 i;
	for(i=0;i<length;i+=2){
		*ptr++=str[i+1];
		*ptr++=str[i];
	}
	*ptr++=0;
	puts(buf);
}

static void sendCommand(u32 base, HDCommand* cmd){
	while(BIT_TEST(io_in8(base+REG_OFFSET_STATUS),7))NICE();
	io_out8(base+REG_OFFSET_DEV_CONTROL,0);
	io_out8(base+REG_OFFSET_FEATURES, cmd->feature);
	io_out8(base+REG_OFFSET_COUNT, cmd->count);
	io_out8(base+REG_OFFSET_LBA_LOW, cmd->lbaLow);
	io_out8(base+REG_OFFSET_LBA_MID, cmd->lbaMid);
	io_out8(base+REG_OFFSET_LBA_HIGH, cmd->lbaHigh);
	io_out8(base+REG_OFFSET_DEVICE, cmd->device);
	io_out8(base+REG_OFFSET_COMMAND, cmd->command);
}

static ASMLINKAGE void irq13_handler(u32 id){
	u32 status=io_in8(REG_BASE_PRIMARY+REG_OFFSET_STATUS);
	//printf("%p",status);
	sendInterrupt(harddisk_proc);
	//putc('.');
}
