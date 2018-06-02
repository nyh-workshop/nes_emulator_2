#include "joypad.h"
#include "nesClassicController.h"

extern nesClassicController ncc;

unsigned char joypad0_value = 0;

// Reference: http://blog.tozt.net/2014/10/15/writing-an-nes-game-part-3/

unsigned char NES_GetJoyPadVlaue(unsigned char JOYPAD_INPUT) {
	
	unsigned char joypad0_returnBit;
    if(JOYPAD_INPUT == 0) {
		joypad0_returnBit = joypad0_value & 0x01;
		joypad0_value = joypad0_value >> 1;	    
		return joypad0_returnBit;	
	}
}

void NES_JoyPadReset(void)
{
	ncc.readButtons();
	joypad0_value |= ncc.button_Right()  << 7;
	joypad0_value |= ncc.button_Left()   << 6;
	joypad0_value |= ncc.button_Down()   << 5;
	joypad0_value |= ncc.button_Up()     << 4;
	joypad0_value |= ncc.button_Start()  << 3;
	joypad0_value |= ncc.button_Select() << 2;
	joypad0_value |= ncc.button_B()      << 1;
	joypad0_value |= ncc.button_A()      << 0;
}
