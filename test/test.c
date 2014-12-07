#include "include/proto.h"
#include "include/type.h"
#include "include/global.h"
// Always include these file!!!
// Otherwise the complier will not give warning,
// but you may receive a #page fault.


int main(){
	struct tm tmm;
	while(1){
		gmtime(wall_clock, &tmm);
		dispByte(BIN2BCD(tmm.month));puts("/");dispByte(BIN2BCD(tmm.day));puts(" ");
		dispByte(BIN2BCD(tmm.hour));puts(":");dispByte(BIN2BCD(tmm.minute));puts(":");dispByte(BIN2BCD(tmm.second));puts(" ");
		delay(1000);
		puts("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
}

// Main function can have whatever return value, 
// but without arguments.
