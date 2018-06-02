#ifndef JOYPAD_H
#define JOYPAD_H

#define JOYPAD_0 0
#define JOYPAD_1 1

unsigned char NES_GetJoyPadVlaue(unsigned char JOYPAD_INPUT);
void NES_JoyPadReset(void);

#endif