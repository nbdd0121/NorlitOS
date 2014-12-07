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
     This file is the part dealing with user interface.
     The path of this source code is kernel/ui.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"
#include "../driver/keyboard/keyboard.h"
#include "../driver/mouse/mouse.h"

static ASMLINKAGE void windowsManager();

INITIALIZER FASTCALL void init_windows(){
	createProcess((u32)windowsManager, 0, "WinManager",0xFF);
}

#define MSG_ADDR 0

static ASMLINKAGE void windowsManager(){
	printStartupTime();
	puts(" WIN: Initialized.\r\n");
	while(1){
		sendMessage(5,0);
		if(receiveMessage(MSG_ADDR)!=-2){
			switch(*((u32*)MSG_ADDR)){
				case EVENT_MOUSE:{
					MOUSE_MSG *ms_msg=(void*)MSG_ADDR;
					puts("No moving.\b\b\b\b\b\b\b\b\b\b");
					break;
				}
				case EVENT_KEYBOARD:{
					KEYBOARD_MSG *kb_msg=(void*)MSG_ADDR;
					if(kb_msg->type)continue;
					u8 output=kb_msg->key;
					if((output>='a'&&output<='z')&&BIT_TEST(kb_msg->led,2))
						output-='a'-'A';
					switch(output){
						case PAD_0...PAD_9:output-=PAD_0-'0';break;
					}
					if(BIT_TEST(kb_msg->shift,BIT_SHIFT_L)||
								BIT_TEST(kb_msg->shift,BIT_SHIFT_R))
						switch(output){
							case 'a'...'z':output-='a'-'A';break;
							case 'A'...'Z':output+='a'-'A';break;
							case '`':output='~';break;
							case '0':output=')';break;
							case '1':output='!';break;
							case '2':output='@';break;
							case '3':output='#';break;
							case '4':output='$';break;
							case '5':output='%';break;
							case '6':output='^';break;
							case '7':output='&';break;
							case '8':output='*';break;
							case '9':output='(';break;
							case '-':output='_';break;
							case '=':output='+';break;
							case '[':output='{';break;
							case ']':output='}';break;
							case '\\':output='|';break;
							case ';':output=':';break;
							case '\'':output='"';break;
							case ',':output='<';break;
							case '.':output='>';break;
							case '/':output='?';break;
							default:break;
						}
					
					puts("\b \b");
					if(output<0x80){
						if(output=='\x7F')puts("  \b\b");
						else if(output=='\r')puts("\r\n");
						else if(output=='\b')puts("\b");
						else putc(output);
					}else{
						switch(output){
						case UP:putc('\1');break;
						case DOWN:putc('\n');break;
						case LEFT:putc('\b');break;
						case RIGHT:putc(' ');break;
						case F12:putc('\f');break;
						case F1:puts("Copyright (C) Gary \x80\x81\x82\x83, All Right Reserved. ");break;
						case VOL_UP:puts("Volumne Up");
						}
					}
					putc('\x84');
				}
			}
		}
	}
}
