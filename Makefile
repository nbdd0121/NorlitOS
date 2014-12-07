##################################
# Makefile for Norlit OS #
##################################

# Entry point of NLOS
# It must have the same value with 'KERNEL_ENTRY' in loader.inc!
ENTRYPOINT	= 0xC0100000

# Commands and flags
ASM		= nasm
CC		= gcc
LD		= ld
ASMFLAGS	= -I boot/include/
ASMLINKAGE	= -f elf -I kernel/include/
CFLAGS		= -I kernel/include/ -c -O2 -g -fno-builtin -fno-leading-underscore -fno-stack-protector -funsigned-char -Wimplicit
C0FLAGS		= -I kernel/include/ -c -g -fno-builtin -fno-leading-underscore -fno-stack-protector -funsigned-char -Wimplicit
LDFLAGS		= -e _start -Ttext $(ENTRYPOINT) -T ldscript.lds

# It must have the same value in loader.inc and judgement.sh and config.h!
HDBOOTSIZE = 1
HDBOOTOFFSET = 4
SMPSIZE	= 1
SMPOFFSET= 5
LOADERSIZE	= 2
LOADEROFFSET= 6
KERNELSIZE	= 80
KERNELOFFSET= 8

ifeq ($(bit),64)
ASMFLAGS += -dBIT64=1
ASMLINKAGE := -f elf64 -dBIT64=1
CFLAGS += -m64 -DBIT64
C0FLAGS += -m64 -DBIT64
LD := ld -melf_x86_64
else
CFLAGS += -m32
C0FLAGS += -m32
LD := ld -melf_i386
endif

# Booter
BOOTER		= boot/boot.bin
HDBOOT		= boot/hdboot.bin
LOADER		= boot/loader.bin
SMPLOADER	= boot/smp.bin
KERNEL		= kernel.bin

DRIVERS		= kernel/bin/driver-rtc.o kernel/bin/driver-apic.o\
				kernel/bin/driver-acpi.o kernel/bin/driver-keyboard.o\
				kernel/bin/driver-mouse.o kernel/bin/driver-video.o\
				kernel/bin/driver-harddisk.o kernel/bin/driver-filesystem.o
KERNELOBJS	= kernel/bin/interrupt.o kernel/bin/start.o kernel/bin/lib.o kernel/bin/global.o\
				kernel/bin/int.o kernel/bin/memory.o kernel/bin/paging.o kernel/bin/proc.o\
				kernel/bin/procasm.o kernel/bin/timer.o kernel/bin/format.o kernel/bin/page.o\
				kernel/bin/windows.o kernel/bin/proclib.o\
				$(DRIVERS)
				

# All Phony Targets
.PHONY : everything bit64 final clean realclean all image run

# Default starting position
everything : $(BOOTER) $(HDBOOT) $(LOADER) $(SMPLOADER) $(KERNEL)

all : realclean everything

final : image clean

clean :
	rm -f $(BOOTER) $(HDBOOT) $(LOADER) $(SMPLOADER) $(KERNELOBJS) kernel.o $(KERNEL)
	rmdir kernel/bin

realclean :
	rm -f $(BOOTER) $(HDBOOT) $(LOADER) $(SMPLOADER) $(KERNELOBJS) $(KERNEL)
	
image : everything
	./judgement.sh
	cp boot/boot.bin nl.img
	dd if=boot/hdboot.bin of=nl.img bs=512 count=$(HDBOOTSIZE) seek=$(HDBOOTOFFSET) conv=notrunc
	dd if=boot/loader.bin of=nl.img bs=512 count=$(LOADERSIZE) seek=$(LOADEROFFSET) conv=notrunc
	dd if=boot/smp.bin of=nl.img bs=512 count=$(SMPSIZE) seek=$(SMPOFFSET) conv=notrunc
	dd if=kernel.bin of=nl.img bs=512 count=$(KERNELSIZE) seek=$(KERNELOFFSET) conv=notrunc

$(BOOTER): boot/boot.asm boot/include/*.inc
	$(ASM) $(ASMFLAGS) -o $@ $<
	
$(HDBOOT): boot/hdboot.asm boot/include/*.inc
	$(ASM) $(ASMFLAGS) -o $@ $<

$(LOADER): boot/loader.asm boot/include/*.inc
	$(ASM) $(ASMFLAGS) -o $@ $<
	
$(SMPLOADER): boot/smp.asm boot/include/*.inc
	$(ASM) $(ASMFLAGS) -o $@ $<

kernel.o: kernel/bin $(KERNELOBJS)
	$(LD) $(LDFLAGS) -o $@ $(KERNELOBJS)
	
kernel/bin:
	mkdir kernel/bin
	
$(KERNEL): kernel.o
	objcopy -O binary $< $@

kernel/bin/interrupt.o: kernel/interrupt.asm boot/include/loader.inc
	$(ASM) $(ASMLINKAGE) -o $@ $<
	
kernel/bin/procasm.o: kernel/process/proc.asm
	$(ASM) $(ASMLINKAGE) -o $@ $<
	
kernel/bin/start.o: kernel/start.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/bin/int.o: kernel/int.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/memory.o: kernel/memory/memory.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/bin/paging.o: kernel/memory/paging.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/proc.o: kernel/process/proc.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/bin/page.o: kernel/process/page.c
	$(CC) $(CFLAGS) -o $@ $<
		
kernel/bin/lib.o: kernel/lib.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/proclib.o: kernel/proclib.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/format.o: kernel/format.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/timer.o: kernel/process/timer.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/global.o: kernel/global.c kernel/include/global.h
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-apic.o: kernel/driver/apic.c kernel/include/apic.h
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-rtc.o: kernel/driver/rtc.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-acpi.o: kernel/driver/acpi.c
	$(CC) $(CFLAGS) -o $@ $< 
	
kernel/bin/driver-keyboard.o: kernel/driver/keyboard/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-mouse.o: kernel/driver/mouse/mouse.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-video.o: kernel/driver/video/video.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-harddisk.o: kernel/driver/harddisk/harddisk.c
	$(CC) $(CFLAGS) -o $@ $<
	
kernel/bin/driver-filesystem.o: kernel/driver/filesystem/filesystem.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/bin/windows.o: kernel/windows.c
	$(CC) $(CFLAGS) -o $@ $<
