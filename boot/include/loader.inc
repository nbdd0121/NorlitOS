; -----------------------------------------------------------------------
;   
;   Copyright 2012-2013 by Gary Guo - All Rights Reserved
;
;   * All source code or binary file can only be used as personal
;     study or research, but can not be used for business.
;   * Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   * Redistributions in binary form must reproduce the above
;     copyright notice, this list of conditions and the following
;     disclaimer in the documentation and/or other materials provided
;     with the distribution.
;     
;     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
;     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
;     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
;     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
;     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
;     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
;     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
;     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
;     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; -----------------------------------------------------------------------
;     This file is the including file of both boot.asm and loader.asm
;     The path of this source code is boot/include/loader.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

LOADER_SEGMENT      equ 0x800
LOADER_OFFSET       equ 0x0
LOADER_PHYADDR      equ LOADER_SEGMENT*0x10+LOADER_OFFSET

KERNELFILE_SEGMENT  equ 0xB00
KERNELFILE_OFFSET   equ 0x0
KERNELFILE_PHYADDR  equ KERNELFILE_SEGMENT*0x10+KERNELFILE_OFFSET
KERNEL_ENTRY        equ 0xC0100000

SMP_SEGMENT			equ 0x900
SMP_OFFSET			equ 0x0
SMP_PHYADDR     	equ SMP_SEGMENT*0x10+SMP_OFFSET

HDBOOT_SEGMENT		equ 0xA00
HDBOOT_OFFSET		equ 0x0
HDBOOT_PHYADDR     	equ HDBOOT_SEGMENT*0x10+HDBOOT_OFFSET

HDBOOT_SIZE_SECT    equ 1
HDBOOT_FILE_LBA     equ 4

LOADER_SIZE_SECT    equ 2
LOADER_FILE_LBA     equ 6

SMP_SIZE_SECT		equ 1
SMP_FILE_LBA		equ 5

KERNEL_SIZE_SECT    equ 80
KERNEL_FILE_LBA     equ 8

TrackPerCylinder    equ 2
SectorPerTrack      equ 18

BootParamAddrSeg    equ 0x50
BootParamPhyAddr    equ BootParamAddrSeg*0x10

SpinLock			equ 0x0
ARDSNum             equ SpinLock+4
ARDSAddrOffset      equ ARDSNum+0x4

LockAddr			equ BootParamPhyAddr+SpinLock

VESAAddrSeg			equ 0x100
VESASign			equ 0
VESAVer				equ VESASign+4
VESAOEM				equ VESAVer+2
VESACap				equ VESAOEM+4
VESAModes			equ VESACap+4
VESAMem				equ VESAModes+4

VESAModeCount		equ VESASign+0x22
VESAEnd				equ VESASign+512


PTEAddr             equ 0x2000
PDEAddr             equ 0x6000
