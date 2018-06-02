#ifndef APU_H
#define APU_H
#include <Arduino.h>
#include <stdbool.h>


// APU Defines from InfoNES/pNESX:
#define SCAN_APU_CLK1   45      // 240Hz
#define SCAN_APU_CLK2   110     // 240Hz 120Hz
#define SCAN_APU_CLK3   176     // 240Hz
#define SCAN_APU_CLK4   241     // 240Hz 120Hz 60Hz


void pNesX_ApuClk_60Hz();
void pNesX_ApuClk_120Hz();
void pNesX_ApuClk_240Hz();
void ApuWrite( unsigned short wAddr, BYTE byData );
BYTE ApuRead( unsigned short wAddr );
void ApuMute(bool mute);
void ApuInit();
unsigned short Rand32k();
unsigned short Rand96();

#endif
