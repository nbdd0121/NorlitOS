加电启动流程
boot.asm
	将loader加载到boot/include/loader.inc:LOADER_PHYADDR位置
	转到Loader执行
loader.asm
	使用BIOS中断服务读取内存信息到boot/include/loader.inc:BootParamPhyAddr+ARDSNum处
	将内核装载到boot/include/loader.inc:KERNELFILE_PHYADDR处
	加载GDT，进入保护模式
	将内核复制到boot/include/loader.inc:KERNEL_ENTRY中
	检查分页相关功能CPU支持情况
	设置16MB的页表
	开启分页
	转到内核

内核初始化流程
interrupt.asm:_start
	设置初始化用途的系统堆栈
	初始化IDT、GDT和TSS
	初始化8259A
	转到start.c:cstart
start.c:start
	清屏
	调用memory.c:init_memory
		显示启动信息
		初始化内存管理
		调用paging.c:init_paging
			初始化内核页表：将前768MB内存映射到0xC0000000上
	调用acpi.c:init_acpi
		将ACPI内存区域映射到0xF0000000上
	调用apic.c:init_apic
		将APIC内存区域映射到0xFEE00000上
		显示APIC信息
	调用proc.c:init_proc
		初始化Init进程
		调用timer.c:init_timer
			调用rtc.c:read_RTC
				获得时间
			设置定时器
		启动进程
		
	
	