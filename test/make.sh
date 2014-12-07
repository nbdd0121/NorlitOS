gcc -c $1 -o main.o -O3 -fno-builtin -fno-leading-underscore -m32
ld -e _start -Ttext 0xC0100000 -T include/ldscript.lds -o kernel.o bin/norlitos.a main.o -melf_i386
objcopy -O binary kernel.o kernel.bin
cp bin/boot.bin norlit.img
dd if=kernel.bin of=norlit.img bs=512 count=30 seek=6 conv=notrunc
rm kernel.bin kernel.o main.o
echo "Thank you for using Norlit OS Beta V0.000000001!"
echo "The output file is norlit.img."
echo "You can use Virtual Machine Softwares, such as VMware, VirtualBox,"
echo " or Hyper-V(change the file name to .vfd if you use hyper-v) to test it!"

