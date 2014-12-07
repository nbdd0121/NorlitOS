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
#include "filesystem.h"

static ASMLINKAGE void norlitfs_bottom();
static void mkfs(u32, u32);

static u32 handle;
static u32 root;

INITIALIZER FASTCALL void init_fs(){
	createProcess((u32)norlitfs_bottom, 0, "NorlitFS",
		DEVICE_DRIVER_PRIORITY);
}

#define MSG_ADDR 0

static ASMLINKAGE void norlitfs_bottom(){
	putc('\f');
	allocMemory(MSG_ADDR, 1);
	u32 psize;
	{
		handle=fileOpen(PID_HD, "$MBR");
		HardDisk* hd=(HardDisk*)fileBuffer(PID_HD, handle);
		u32 part;
		u8 partName[3];
		for(part=0;part<hd->partCount;part++){
			if(BIT_CLEAR(hd->part[part].type,31)==MAGIC_NORLIT){
				printStartupTime();
				sprintf(partName,"%d",part+1);
				printf(" FS: Norlit Partition Found: //Device/HD0/%s.\r\n", partName);
				psize=hd->part[part].size;
				break;
			}
		}
		if(part==hd->partCount){
			printStartupTime();
			puts(" FS: No Norlit Partition(System ID 6E) Found.\r\n");
			EXIT();
		}
		handle=fileOpen(PID_HD, partName);	/* Opens the partition */
	}/* The above code finds out the Norlit FS's partition */
	{
		u32 hdFlag=*(u8*)HDBOOT_OFFSET==0;
		SuperBlock* spb=(SuperBlock*)fileRead(PID_HD, handle, 1, 1);
		hdFlag=1;
		if(hdFlag){
			printStartupTime();
			if(memcmp(spb->signature,"NorlitFS",8)==0){
				puts(" FS: File System Found.\r\n");
			}else{
				puts(" FS: No File System Found.\r\n");
				EXIT();
			}
		}else{
			mkfs(handle, psize);
			fileRead(PID_HD, handle, 1, 1);
		}
		psize=spb->size;
		printStartupTime();
		printf(" FS: Total: %dKB, Free: %dKB, Used: %dKB\r\n",
				spb->size*2, spb->freeSize*2, (spb->size-spb->freeSize)*2);
		root=spb->root;
	}/* Reads the super block, and mkfs if FS does not exist. */

	/**********************************************************
	 * The following part is `real` file system driver
	 **********************************************************/
	
	FileMessage* msg=(FileMessage*)MSG_ADDR;
	while(1){
		//NICE();
		s32 proc=receiveMessage(MSG_ADDR);
		if(msg->signature!=COMMAND_HD)continue;
		switch(msg->command){
			case HD_OPEN:{
				if(msg->data[0]!='/')break;
				printStartupTime();
				puts(" FS: Open ");
				u8 c=0;
				u8* str=msg->data+1;
				u8* ptr=str;
				u32 dirAddr=root;
				if(*ptr!='\0')while(1){
					c=*ptr++;
					if(c=='/'||c=='\0'){
						*(ptr-1)=0;
						u32 hash=stringHash(str);
						dirAddr=seekDataBlock(dirAddr, 0);
						putc('/');
						puts(str);
						DirectoryBlock* db=(DirectoryBlock*)fileRead(PID_HD, handle, dirAddr*4, 4);
						FileEntry* filePtr;
						for(filePtr=db->filePtr;filePtr->address;filePtr++){
							if(filePtr->hash==hash)break;
						}
						
						if(!filePtr->address){
							puts("(created)");
							u32 fb=createFileBlock(str);
							fileRead(PID_HD, handle, dirAddr*4, 4);
							filePtr->address=fb;
							filePtr->hash=hash;
							fileWrite(PID_HD, handle, dirAddr*4, 4);
							dirAddr=fb;
						}else{
							dirAddr=filePtr->address;
						}
						if(c==0)break;
						str=ptr;
					}
				}else putc('/');
				if(c)break;
				puts("\r\n");
				memcpy(msg->data, fileRead(PID_HD, handle, dirAddr*4, 4), 2048);
				msg->handle=dirAddr;
				sendMessage(proc, MSG_ADDR);
				break;
			}
			case HD_READ:{
				u32 cls=seekDataBlock(msg->handle, msg->offset);
				memcpy(msg->data, fileRead(PID_HD, handle, cls*4, 4), 2048);
				sendMessage(proc, MSG_ADDR);
				break;
			}
			case HD_WRITE:{
				u32 cls=seekDataBlock(msg->handle, msg->offset);
				memcpy(fileBuffer(PID_HD, handle), msg->data, 2048);
				fileWrite(PID_HD, handle, cls*4, 4);
				sendMessage(proc, MSG_ADDR);
				break;
			}
		}
	}
}
static void mkfs(u32 handle, u32 psize){
	u32 ksize=(KERNEL_SIZE-1)/(SECTOR_SIZE*4)+1;
	printStartupTime();
	puts(" FS: Copying Files  0%");
	u8* data=fileBuffer(PID_HD, handle);
	{
		memcpy(data, (void*)HDBOOT_OFFSET, 512);
		memcpy(data+512, (void*)SMP_LOADER_OFFSET, 512);
		memcpy(data+1024, (void*)LOADER_OFFSET, 1024);
		fileWrite(PID_HD, handle, 4, 4);
		printf("\b\b\b%2d%",100/(ksize+2));
		memcpy(fileRead(PID_HD, 0, 0, 1), (void*)HDBOOT_OFFSET, 446);
		fileWrite(PID_HD, 0, 0, 1);
		printf("\b\b\b%2d%",200/(ksize+2));
		u32 i;u8* ker=(u8*)(0xB000+PAGE_OFFSET);
		for(i=0;i<ksize;i++){
			memcpy(data, ker, SECTOR_SIZE*4);
			ker+=SECTOR_SIZE*4;
			fileWrite(PID_HD, handle, 4*i+8, 4);
			printf("\b\b\b%2d%",(i+3)*100/(ksize+2));
		}
		puts("\r\n");
	}/* This part write the kernel file into the harddisk */
	
	SuperBlock* sb=(SuperBlock*)data;
	memcpy(sb->signature,"NorlitFS",8);
	sb->size=psize/4;
	u32 i;
	for(i=0;i<20;i++){
		sb->freeBlock[i]=0;
	}
	fileWrite(PID_HD, handle, 1, 1);
	
	{
		u32 address=ksize+2;
		u32 size=psize/4-address;
		u8 len;
		printStartupTime();
		puts(" FS: Formatting  0%");
		for(len=0;len<20;len++){
			u32 sizep=1<<len;
			printf("\b\b\b%2d%",len*2*100/43);
			if(size<sizep)break;
			if(address&sizep){
				// address ~ sizep
				//printf("[%x-%x]",address, address+sizep);
				freeBlock(address, len);
				address+=sizep;
				size-=sizep;
			}
			printf("\b\b\b%2d%",(len*2+1)*100/43);
			if(size&sizep){
				// address+size-sizep ~ sizep
				//printf("[%x-%x]",address+size-sizep, address+size);
				freeBlock(address+size-sizep, len);
				size-=sizep;
			}
		}
		printf("\b\b\b%2d%",41*100/43);
	}
	
	u32 rootBlock=createFileBlock("$Root");
	printf("\b\b\b%2d%",42*100/43);
	fileRead(PID_HD, handle, 1, 1);
	sb->root=rootBlock;
	fileWrite(PID_HD, handle, 1, 1);
	puts("\b\b\b100%\r\n");
	printStartupTime();
	puts(" FS: Norlit OS Installed.\r\n");
}

static u32 seekDataBlock(u32 file, u32 offset){
	FileBlock* fb=(FileBlock*)fileRead(PID_HD, handle, file*4, 4);
	u32 i;
	for(i=0;i<DATA_BLOCK_PTRS;i++){
		if(fb->dataPtr[i].size){
			if(offset<fb->dataPtr[i].size){
				return fb->dataPtr[i].address;
			}else{
				offset-=fb->dataPtr[i].size;
			}
		}else{
			break;
		}
	}
	return 0;
}

static u32 createFileBlock(u8* name){
	u32 blk=mallocBlock(0);
	if(blk==0){
		printStartupTime();
		puts(" FS: No More Space Avaliable.\r\n");
		return 0;
	}
	u32 dataBlock=mallocBlock(0);
	DataBlock* db=(DataBlock*)fileBuffer(PID_HD, handle);
	memset(db, 0, 2048);
	fileWrite(PID_HD, handle, dataBlock*4, 4);
	FileBlock* fb=(FileBlock*)db;
	strcpy(fb->fileName, name);
	fb->nameHash=stringHash(name);
	fb->size=0;
	fb->attributes=0;
	fb->dataPtr[0].address=dataBlock;
	fb->dataPtr[0].size=1;
	fb->dataPtr[1].address=0;
	fb->dataPtr[1].size=0;
	fileWrite(PID_HD, handle, blk*4, 4);
	return blk;
}
static u32 mallocBlock(u32 len){
	SuperBlock* sb=(SuperBlock*)fileRead(PID_HD, handle, 1, 1);
	u32 ret=sb->freeBlock[len];
	if(ret==0){
		u32 l2;
		for(l2=len+1;l2<20;l2++){
			if(sb->freeBlock[l2]){
				u32 rt=sb->freeBlock[l2];
				u32 nxt=*(u32*)fileRead(PID_HD, handle, rt*4, 1);
				fileRead(PID_HD, handle, 1, 1);
				sb->freeBlock[l2]=nxt;
				sb->freeSize-=1<<l2;
				fileWrite(PID_HD, handle, 1, 1);
				for(l2--;(s32)l2>=(s32)len;l2--){
					freeBlock(rt+(1<<l2),l2);
				}
				return rt;
				break;
			}
		}
		return 0;
	}else{
		u32 nxt=*(u32*)fileRead(PID_HD, handle, ret*4, 1);
		fileRead(PID_HD, handle, 1, 1);
		sb->freeBlock[len]=nxt;
		sb->freeSize-=1<<len;
		fileWrite(PID_HD, handle, 1, 1);
		//printf("%d-%d", ret,nxt);
		return ret;
	}
}

static void freeBlock(u32 base, u32 len){
	SuperBlock* sb=(SuperBlock*)fileRead(PID_HD, handle, 1, 1);
	u32 cid=sb->freeBlock[len];
	sb->freeSize+=1<<len;
	sb->freeBlock[len]=base;
	fileWrite(PID_HD, handle, 1, 1);
	*(u32*)fileBuffer(PID_HD, handle)=cid;
	fileWrite(PID_HD, handle, base*4, 1);
}
