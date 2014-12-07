s=`du -b boot/loader.bin|awk '{print $1}'`
if [ $s -gt `expr 5 "*" 512` ]	# 如果LOADER的大小过大，就要发出警告。
then
echo ">>>PANIC<<< The Size of LOADER is over 2560, please change the size inside loader.inc immediately";
exit 1
fi
s=`du -b kernel.bin|awk '{print $1}'`
if [ $s -gt `expr 80 "*" 512` ]	# 如果KERNEL的大小过大，就要发出警告。
then
echo ">>>PANIC<<< The Size of KERNEL is over 20480, please change the size inside loader.inc immediately";
exit 1
fi
