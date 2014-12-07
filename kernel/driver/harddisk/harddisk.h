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

#define MBR_LOCATION	0x1BE

#define REG_BASE_PRIMARY       0x1F0
#define REG_BASE_SECONDARY     0x170

#define REG_OFFSET_DATA        0x0   /* R/W */
#define REG_OFFSET_ERROR       0x1   /* Read Only */
#define REG_OFFSET_FEATURES    0x1   /* Write Only */
#define REG_OFFSET_COUNT       0x2   /* R/W */
#define REG_OFFSET_LBA_LOW     0x3   /* R/W */
#define REG_OFFSET_LBA_MID     0x4   /* R/W */
#define REG_OFFSET_LBA_HIGH    0x5   /* R/W */
#define REG_OFFSET_DEVICE      0x6   /* R/W */
#define REG_OFFSET_STATUS      0x7   /* Read Only */
#define REG_OFFSET_COMMAND     0x7   /* Write Only */
#define REG_OFFSET_ALT_STATUS  0x206 /* Read Only */
#define REG_OFFSET_DEV_CONTROL 0x206 /* Write Only */

#define MASTER			0xA0
#define SLAVE			0xB0
#define LBA				0b1000000
#define MBR_CHECK		0xAA55

#define ATA_IDENTIFY	0xEC
#define ATA_READ		0x20
#define ATA_WRITE		0x30

#pragma pack(1)
typedef struct{
	u8 feature;
	u8 device;
	u8 count;
	u8 lbaLow;
	u8 lbaMid;
	u8 lbaHigh;
	u8 command;
}HDCommand;

typedef struct{
	u8 active;
	u8 startHead;
	u16 startSectorAndCylinder;
	u8 fileSystem;
	u8 endHead;
	u16 endSectorAndCylinder;
	u32 location;
	u32 size;
}MBREntry;

typedef struct{
	u8 code[MBR_LOCATION];
	MBREntry entry[4];
	u16 endCheck;
}MBRTable;

static Process* harddisk_proc;
static u8 diskAmount=0;

static void sendCommand(u32, HDCommand* );
static void printIdentifyStr(u8* str, u32 length);
static u32 identifyHD(HardDisk* hd);
static ASMLINKAGE void harddisk_bottom();
static ASMLINKAGE void irq13_handler(u32);
