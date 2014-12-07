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
;     This file is the including file for protect mode
;     The path of this source code is boot/include/protect.asm
;     Gary Guo, 2013, All right reserved.
; -----------------------------------------------------------------------

;=====================================================
; 段选择子属性常量 Segment Selector Attribute, SSA
;-----------------------------------------------------
SSA_RPL0    EQU 0	; RPL = 0
SSA_RPL1    EQU 1	; RPL = 1
SSA_RPL2    EQU 2	; RPL = 2
SSA_RPL3    EQU 3	; RPL = 3

SSA_TIG     EQU 0	; TI = GDT
SSA_TIL     EQU 4	; TI = LDT
;-----------------------------------------------------

;=====================================================
; 段描述符属性常量 Segment Descriptor Attribute，SDA
;-----------------------------------------------------
SDA_L       EQU 2000h	; 64位标志
SDA_D       EQU 4000h	; D/B 位设置为32位
SDA_G       EQU 8000h	; G位颗粒度设置为 4KB
SDA_P       EQU 80h		; P位设置为存在
SDA_S       EQU 10h		; S为设置为代码数据段
;-----------------------------------------------------
; 特权级
;-----------------------------------------------------
SDA_DPL0    EQU 00h	; DPL = 0
SDA_DPL1    EQU 20h	; DPL = 1
SDA_DPL2    EQU 40h	; DPL = 2
SDA_DPL3    EQU 60h	; DPL = 3
;-----------------------------------------------------
; 存储段描述符类型值说明
;-----------------------------------------------------
SDA_A       EQU 1h	; 已访问
SDA_R_W     EQU 2h  ; 可写数据段/可读代码段
SDA_CO      EQU 4h	; 一致代码段/向下扩展数据段
SDA_C       EQU 8h	; 可执行（代码段）
; 组合说明
; 数据段：必须能读,不允许设置SDA_C，SDA_R_W控制是否能写，SDA_CO控制是否是向下扩展。
; 代码段：必须能执行，必须设置SDA_C，SDA_R_W控制是否能读，SDA_CO控制是否一致。
SDA_FLAT_C  EQU SDA_G|SDA_D|SDA_P|SDA_S|SDA_C|SDA_R_W	; 平坦代码段
SDA_FLAT_D  EQU SDA_G|SDA_D|SDA_P|SDA_S|SDA_R_W			; 平坦数据段
SDA_FLAT_CL EQU SDA_G|SDA_L|SDA_P|SDA_S|SDA_C|SDA_R_W	; 平坦64位代码段
SDA_FLAT_DL EQU SDA_G|SDA_L|SDA_P|SDA_S|SDA_R_W			; 平坦64位数据段

;-----------------------------------------------------
; 系统段描述符类型值说明
;-----------------------------------------------------
DA_LDT      EQU 82h	; 局部描述符表段类型值
DA_TaskGate EQU 85h	; 任务门类型值
DA_386TSS   EQU 89h	; 可用 386 任务状态段类型值
DA_386CGate EQU 8Ch	; 386 调用门类型值
DA_386IGate EQU 8Eh	; 386 中断门类型值
DA_386TGate EQU 8Fh	; 386 陷阱门类型值
;-----------------------------------------------------

;=====================================================
; SegmentDescriptor(u32 base, u20 limit, u12 attr);初始化段描述符
;-----------------------------------------------------
; Entry:
;	- arg0 -> 段基址
;	- arg1 -> 段界限
;	- arg2 -> 属性
; Exit:
;	- 填充一个段描述符
%macro SegmentDescriptor 3
	dw %2 & 0FFFFh							; 段界限第16位
	dw %1 & 0FFFFh							; 段基址第24位
	db (%1 >> 16) & 0FFh					; 同上
	dw ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh)	; 扩展字节
	db (%1 >> 24) & 0FFh					; 段基址高8位
%endmacro

;=====================================================
; GateDescriptor(u32 offset, u16 selector, u8 count, u8 attr);初始化门描述符
;-----------------------------------------------------
; Entry:
;	- arg0 -> 偏移
;	- arg1 -> 选择子
;	- arg2 -> 计数器
;	- arg3 -> 属性
; Exit:
;	- 填充一个段描述符
%macro GateDescriptor 4
	dw %1 & 0FFFFh							; 门偏移低16位
	dw %2									; 选择子
	dw (%3 & 1Fh) | ((%4 << 8) & 0FF00h)	; 计数器及属性
    dw (%1 >> 16) & 0FFFFh					; 门偏移高16位
%endmacro

;=====================================================
; GDTPointer(u32 base, u16 limit);初始化GDT指针
;-----------------------------------------------------
; Entry:
;	- arg0 -> GDT基址
;	- arg1 -> GDT界限
; Exit:
;	- 填充一个GDT指针
%macro GDTPointer 2
	dw %2 - 1								; GDT界限
	dd %1									; GDT基址
%endmacro

;=====================================================
; IDTPointer(u32 base, u16 limit);初始化IDT指针
; 使用完全与GDTPointer相同
%define IDTPointer GDTPointer

;=====================================================
; InitTSS(u32 esp0, u16 ss0);初始化TSS
;-----------------------------------------------------
; Entry:
;	- arg0 -> ss0
;	- arg1 -> esp0
; Exit:
;	- 填充一个TSS
%macro InitTSS 2
	dw 0,0	; backlink
tss_esp0:
	dd %2	; esp0
	dw %1,0	; ss0
	dd 0	; esp1
	dw 0,0	; ss1
	dd 0	; esp2
	dw 0,0	; ss2
	dd 0	; cr3
	dd 0	; eip
	dd 0	; flags
	dd 0	; eax
	dd 0	; ecx
	dd 0	; edx
	dd 0	; ebx
	dd 0	; esp
	dd 0	; ebp
	dd 0	; esi
	dd 0	; edi
	dw 0,0	; es
	dw 0,0	; cs
	dw 0,0	; ss
	dw 0,0	; ds
	dw 0,0	; fs
	dw 0,0	; gs
	dw 0,0	; ldt
	dw 0	; trap
	dw 104	; iobase
%endmacro
