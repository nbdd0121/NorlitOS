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
     This file is the prototype header.
	 All C files in kernel should use the types defined by
		this file, in order to make use of symbols.
     The path of this source code is kernel/include/proto.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "type.h"
#include "const.h"

/**** General Declarations ****/

/* lib.c */
ASMLINKAGE void putc(u8);
ASMLINKAGE void puts(const u8*);
ASMLINKAGE void dispByte(u8);
ASMLINKAGE void dispInt(u32);
ASMLINKAGE void* memcpy(void*, const void*, u_addr);
ASMLINKAGE void* memset(void*, u8, u_addr);
FASTCALL u8 BCD2BIN(u8 val);
FASTCALL u8 BIN2BCD(u8 val);
FASTCALL u64 do_divmod64(u64, u64, u64*);
FASTCALL u64 do_mod64(u64 num, u64 den);
FASTCALL u64 do_div64(u64 num, u64 den);
FASTCALL u64 mktime(u32, u8, u8, u8, u8, u8);
FASTCALL void gmtime(u64, struct tm*);
FASTCALL void delay(u_addr millisec);
#define BIT_TEST(x,bit) ((x)&(1<<(bit)))

/* memory.c */
FASTCALL void memory_block_free(u_addr address, u8 bit);
FASTCALL void memory_free_nocheck(u_addr, u_addr);
FASTCALL void memory_free(u_addr, u_addr);
FASTCALL void* memory_alloc(u8);
FASTCALL void* malloc(u_addr);
FASTCALL void free(void*);

/* paging.c */
#define ALLOC_PAGE() memory_alloc(12)
#define FREE_PAGE(addr) memory_block_free((u_addr)(addr), 12)
#define ALLOC_PAGES(num) memory_alloc(12+(num))	//	Allocate 2^num
#define FREE_PAGES(addr, num) memory_block_free((u_addr)(addr), 12+(num))	//	Free 2^num
#define va2pa(x) ((u_addr)(x)-PAGE_OFFSET)
#define pa2va(x) ((u_addr)(x)+PAGE_OFFSET)

/* driver-rtc.c */
FASTCALL u8 CMOS_read(u_addr);
FASTCALL void CMOS_write(u_addr, u8);
FASTCALL void write_RTC(u64);
FASTCALL u64 read_RTC();


