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
     This file is the type define header.
	 All C files in kernel should use the types defined by
		this file, in order to keep the cross-platform ability.
     The path of this source code is kernel/include/typedef.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include <stdint.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
typedef int64_t		s64;

typedef u8			boolean;

#define NULL ((void*)0)
#define EMPTY ((u32*)0xFFFFFFFF)
#define TRUE ((boolean)1)
#define FALSE ((boolean)0)

#define ASMLINKAGE __attribute__((regparm(0)))
#define FASTCALL __attribute__((regparm(3)))
#define INLINE inline __attribute__((always_inline))
#define INITIALIZER __attribute__ ((section (".init_text")))

#define BREAKPOINT() do{__asm__ __volatile__("int $3");}while(0)
#define MEMORY_BARRIER() asm volatile("":::"memory")

#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)

#define DEBUG(x) 
