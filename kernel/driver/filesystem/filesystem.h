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
     This file is the constant definition header.
	 This file is for hard disk.
     The path of this source code is kernel/driver/harddisk/harddisk.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/
 
#pragma once

#include "config.h"
#include "hd.h"

#pragma pack(1)
typedef struct{
	u32 address;
	u32 size;
}DataBlockPointer;

typedef struct{
	u8 signature[8]; /* "NorlitFS" */
	u32 size;	/* In 2KB */
	u32 root;
	u32 freeSize;
	u32 freeBlock[20];
	/* (1,2,4,8,16,32,64,128,256)*2KB  ---9
	(1,2,4,8,16,32,64,128,256,512)*1MB ---10
	1GB	*/
	
}SuperBlock;

typedef struct{
	u8 fileName[FILE_NAME_SIZE];
	u32 nameHash;
	u32 size;	/* In Bytes */
	u32 attributes; /* Bit 0 for Directory */
	u8 metaData[META_DATA_LENGTH];
	u8 reserved[2048-DATA_BLOCK_PTRS*sizeof(DataBlockPointer)-sizeof(u32)*3-FILE_NAME_SIZE-META_DATA_LENGTH];
	DataBlockPointer dataPtr[DATA_BLOCK_PTRS];
}FileBlock;

typedef struct{
	u8 data[2048];
}DataBlock;

typedef struct{
	DataBlockPointer dataPtr[EXT_DATA_BLOCKS];
}ExtendBlock;

typedef struct{
	u32 address;
	u32 hash;
}FileEntry;

typedef struct{
	FileEntry filePtr[DIR_FILE_PTRS];
}DirectoryBlock;

static u32 createFileBlock(u8*);
static u32 seekDataBlock(u32 file, u32 offset);
static void freeBlock(u32 base, u32 len);
static u32 mallocBlock(u32 len);
