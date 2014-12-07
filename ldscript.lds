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
.debug_info      0 : { *(.debug_info)  }
.debug_line 0 : { *(.debug_line)  }
.debug_abbrev   0 : { *(.debug_abbrev)}
.debug_frame    0 : { *(.debug_frame) }
.debug_str    0 : { *(.debug_str) }
.debug_loc    0 : { *(.debug_loc) }
.debug_range    0 : { *(.debug_range) }
.debug_arrange    0 : { *(.debug_arrange) }
/DISCARD/ : { *.* }
}
/* 紧凑内存结构 */
