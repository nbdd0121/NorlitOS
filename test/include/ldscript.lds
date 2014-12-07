SECTIONS
{
. = 0x0;
.text : { *(.text*) }
. = ALIGN(0x8);
.data : { *(.data) }
. = ALIGN(0x8);
.rodata : {
	*(.rodata*)
}
. = ALIGN(0x8);
.bss : {
	*(.bss)
	*(COMMON)
}
. = ALIGN(0x8);
.init_text : {
	init_text_seg_start = . ;
	*(.init_text)
}
/DISCARD/ : { *.* }
}
/* 紧凑内存结构 */