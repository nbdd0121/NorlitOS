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
     This file defines types used in kernel.
	 All C files in kernel should use the types defined by
		this file, in order to keep the cross-platform ability.
     The path of this source code is kernel/include/typedef.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "const.h"

#pragma pack(1)

/**
 * LinkedList is widely use in the data structures which need to use
 * a linked list. (Notice that LinkedList should be initialized before
 * they are used.)
 */
typedef struct list_head{
	struct list_head *prev, *next;
}LinkedList;

/**
 * GDTItem defines the structure of a GDT. You can cast it to u64.
 */
typedef struct{
	u16 limitLow;
	u16 baseLow;
	u8  baseMiddle;
	u16 attributes;
	u8  baseHigh;
}GDTItem;

/**
 * IDTItem defines the structure of a IDT. You can cast it to u64.
 */
typedef struct{
	u16 offsetLow;
	u16 selector;
	u16 attributes;
	u16 offsetHigh;
}IDTItem;

/** 
 * StackFrame is the data type to store the registers of a thread.
 * the order of gs, fs, es, ds should be equal to the order to push
 * and pop them in proc.asm. The order of general purpose registers
 * and ignored sp should have the same order as pushad and popad.
 * returnAddress should have the same size as the ip pushed by call.
 * ip, cs, flags, sp and ss should have the same order as iretd.
 */
typedef struct{
	u32 gs, fs, es, ds;
	u32 di, si, bp, ignored;
	u32 bx, dx, cx, ax;
	u32 returnAddress;
	u32 ip, cs, flags, sp, ss;
}StackFrame;

/* This typedef put here because of cross-reference*/
typedef struct str_proc Process;

/**
 * Thread is a multiple-page-sized structure. This structure is also
 * the kernel stack of the thread. (Notice that the first one should
 * be list because proc.asm use it.)
 */
typedef struct{
	LinkedList list;
	Process* process;
	u32 threadID;
	u32 flag;
	u32 stacktop;
	u8 stack[PAGE_SIZE*(1<<THREAD_STRUCT_SIZE)-
		sizeof(LinkedList)-sizeof(Process*)-sizeof(u32)-sizeof(u32)-
		sizeof(u32)-sizeof(StackFrame)];
	StackFrame regs;
}Thread;

/**
 * Process is the structure to describe a process. (Notice that
 * cr3 should always be first one because the code in proc.asm
 * use that.)
 */
struct str_proc{
	u32	cr3;
	Thread* current;
	LinkedList list;
	LinkedList thread;
	u32 avl_thread;
	u32 flag;
	s32 ticks;
	u32 priority;
	u32 pid;
	u32 privilege;
	u32 receiving;
	
	u64* msgHead;
	u64* msgTail;
	u64 msgCount;
	u64 messages[MAX_MESSAGE_COUNT];
	
	u8 name[PROCESS_NAME_LENGTH];
};

/**
 * ARDSItem describes the type of a certain memory interval. The
 * structure should be as the same as the structure returned by the
 * BIOS interrupt.
 */
typedef struct{
	u64 base;
	u64 limit;
	u32 type;
}ARDSItem;

/**
 * ReadModeFarPointer describes a far pointer in real mode. The far
 * pointer is used by VESA BIOS Extension.
 */
typedef struct{
	u16 offset;
	u16 seg;
}ReadModeFarPointer;

/**
 * BootParam is the parameter passed by loader. However, to be convient
 * the lock used by SMP booting is also putted here, though it have
 * no relationship with boot parameters. (Notice that the VGA boot
 * parameters are stored seperatedly.
 */
typedef struct{
	u32 lock;
	u32 len;
	ARDSItem items[0];
}BootParam;

/** IN DEVELOPING
 * VESAModeInfo is the first 50 bytes returned by VESA BIOS Extension
 * 4F01h for a mode both supported by Video Card and by Norlit OS. The
 * verification of a certain mode is in loader.asm.
 * @ref VESA BIOS EXTENSION (VBE) Core Functions Standard 2.0
 */
typedef struct{
	u16 attributes;
	u8 winA,winB;
	u16 granularity;
	u16 winsize;
	u16 segmentA, segmentB;
	
	/* In VBE Specification, this field should be
	 * ReadModeFarPointer winPosFunc; 
	 * However, we overwrite this field in loader n*/
	u16 mode;
	u16 reserved2;
	
	u16 bytesPerLine;
 
	u16 width, height;
	u8 Wchar, Ychar, planes, bitsPerPixel, banks;
	u8 memory_model, bank_size, image_pages;
	u8 reserved0;
 
	u8 red_mask, red_position;
	u8 green_mask, green_position;
	u8 blue_mask, blue_position;
	u8 rsv_mask, rsv_position;
	u8 directcolor_attributes;

	u32 physbase;  // your LFB (Linear Framebuffer) address ;)
	u32 offscreen;
	u16 offsize;
}VESAModeInfo;

/**
 * VESAControllerInfo is the data pack returned by VESA BIOS Extension
 * 4F00h and the appended mode information by loader.asm. Norlit OS
 * occupied a reserved field for modeCount usage.
 * @ref VESA BIOS EXTENSION (VBE) Core Functions Standard 2.0
 */
typedef struct{
	u8 signature[4];
	u8 minorVersion;
	u8 majorVersion;
	ReadModeFarPointer oemString;
	u32 capabilities;
	ReadModeFarPointer videoModes;
	u16 totalMemory;
	u16 OEMVersion;
	ReadModeFarPointer vendor;
	ReadModeFarPointer product;
	ReadModeFarPointer revision;
	/* In VBE Specification, this field should be reserved.
	 * However, we overwrite this field in loader */
	u16 modeCount;
	u8 reserved0[220];
	u8 oemUse[256];
	VESAModeInfo modeList[0];
}VESAControllerInfo;

/**
 * Time structure is output structure of gmtime function. This type
 * contains human-readable time.
 */
typedef struct{
	u32 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u16 milli;
	u8 weekday;
}Time;

#ifdef USE_ACPI_TYPES
	struct ACPI_RSDP{
		u8 signature[8];
		u8 checksum;
		u8 oem_id[6];
		u8 revision;
		u32 rsdt;
		u32 length;
		u64 xsdt;
		u8 ext_checksum;
		u8 reserved[3];
	};

	struct ACPIHeader{
		u32 signature;
		u32 length;
		u8 revision;
		u8 checksum;
		u8 oem_id[6];
		u8 oem_table_id[8];
		u32 oem_revision;
		u32 creator_id;
		u32 creator_revision;
	};

	struct ACPI_RSDT{
		struct ACPIHeader header;
		u32 entry[0];
	};

	struct ACPI_XSDT{
		struct ACPIHeader header;
		u64 entry[0];
	};

	struct ACPI_APICHeader{
		u8 type;
		u8 length;
	};

	struct ACPI_MADT{
		struct ACPIHeader header;
		u32 apic_address;
		u32 flag;
		struct ACPI_APICHeader apic_str[0];
	};

	struct ACPILocalAPIC{
		u8 type;
		u8 length;
		u8 apic_pro_id;
		u8 apic_id;
		u32 flags;
	};

	struct ACPI_IO_APIC{
		u8 type;
		u8 length;
		u8 io_apic_id;
		u8 reserved;
		u32 io_apic_addr;
		u32 int_base;
	};

	struct ACPIIntOverride{
		u8 type;
		u8 length;
		u8 bus; /* Always 0 */
		u8 source;
		u32 interrupt;
		u8 flags;
	};
#endif

/**
 * IntHandler and ExceptionHandler are handler types for interrupt
 * dealing. The difference is that Exception Handler contains two
 * parameter, one of them contains error code.
 */
typedef ASMLINKAGE void (*IntHandler)(u32);
typedef ASMLINKAGE void (*ExceptionHandler)(u32,u32);

/**
 * TSS is Intel-defined structure. However, we do not use hard task
 * switch, so only ss0 and esp0 are used. Others are reserved.
 */
typedef struct{
	u32 reserved0;
	u32 esp0;
	u16 ss0;
	u16 reserved1;
	u32 reserved2[23];
}TSS;

/**
 * KernelPage is the page allocated to different processor. This page
 * will not be shared by different processors. (Notice that GDTItem, TSS,
 * and GDT Pointer (GDTLimit and GDTBase) should be 16-byte-aligned.
 * Also notice that TSS is 0x68-bytes-long.)
 */
typedef struct{
	Process* current_proc;
	Thread* current_thread;
	u32 free_memory_counter;
	u32 cpuid;
	// 16 byte aligned.
	GDTItem GDTTable[GDT_LENGTH];
	ExceptionHandler handlerTable[0x20];
	IntHandler handlerTable2[IDT_LENGTH-0x20];
	TSS	tss;
	void* kernelStack;
	//u16 reserved1;
	//u32* memoryBlocks[28];
}KernelPage;

typedef struct{
	u32 signature;
	u8 data[512-sizeof(u32)];
}Message;

typedef struct{
	u32 signature;
	u32 command;
	u32 handle;
	u32 offset;	/* Measures in sector */
	u32 count;	/* Measures in sector */
	u8 reserved0[512-sizeof(u32)*5];
	u8 reserved1[PAGE_SIZE-2048-512];
	u8 data[2048];
}FileMessage;

typedef struct{
	u8 fileName[FILE_NAME_SIZE];
	u32 nameHash;
	u32 size;	/* In Bytes */
	u32 attributes; /* Bit 0 for Directory */
	u8 metaData[META_DATA_LENGTH];
	u8 reserved[2048-sizeof(u32)*3-FILE_NAME_SIZE-META_DATA_LENGTH];
}MetaData;
