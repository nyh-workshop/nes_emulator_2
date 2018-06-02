// NES Emulator (2) for Chipkit series (PIC32MZ).
// version 1.0
// System requirements:
// 1.) At least 256K RAM
// 2.) Processor speed: 200MHz
// 3.) NES Mini Controller (I2C)
// Refer to the Github page for the wiring diagrams and etc.
// References:
// Credits to the guys from OpenEDV : http://www.openedv.com/thread-266483-1-1.html (NES core)
// and the pNESX author too: https://os.mbed.com/users/beaglescout007/code/Nucleo_Ex06_EMU/ (NES sound driver)

#include "Arduino.h"
#include "nes_main.h"
#include "ili9341_spi.h"
#include "sys/kmem.h"
#include <sys/attribs.h>

void __USER_ISR dma0isr(void) {

  DCH0INTbits.CHBCIF = 0;
  IFS4bits.DMA0IF = 0;

}

void setup() {

  Serial.begin(9600);
  Serial.println("testing...");
  TRISDbits.TRISD4 = 0;
  PORTDbits.RD4 = 0;
  // put your setup code here, to run once:
  //nes_clock_set(16);      //ÉèÖÃÎª128M ³¬ÆµÁËµÄ Î£ÏÕ
  ILI9341_begin();

  ILI9341_setRotation(3); // 0 = portrait mode.
  // 1 = landscape mode.
  // 2 = portrait mode (180 degrees rotated).
  // 3 = landscape mode (180 degrees rotated).

  // Self explanatory:
  ILI9341_clrScreen();

  // Self explanatory:
  ILI9341_write_txt_40x25(0, 0, 0x01, 0x0f, (unsigned char*)" NES Emulator               ");
  ILI9341_write_txt_40x25(0, 3, 0, 0x0f, (unsigned char*)"Ported to Chipkit, 2016-2018 uncle-yong");
  ILI9341_write_txt_40x25(0, 5, 0, 0x0f, (unsigned char*)"Loading ROM...");
  ILI9341_write_screen_40x25();

  // Show title screen for 2 seconds:
  delay(2000);

  // Then wipe out the screen.
   ILI9341_clrScreen();

  // The screen is readjusted and the DMA channels are reconfigured
  // so that the screen is continuously drawn without interruption:
   ILI9341_setAddrWindow(32, 8, 256 + 32 - 1, 240 + 8 - 1);
   
  _dc = 1;
  _cs = 0;

  //nes_clock_set(16);      //ÉèÖÃÎª128M ³¬ÆµÁËµÄ Î£ÏÕ

  // Configure DMA1 for bottom half of the screen and channel enable.
  DCH1CON = 0x0000;
  DCH1INTCLR = 0x00ff;
  DCH1CONbits.CHCHN = 1;
  DCH1ECONbits.CHSIRQ = _SPI1_TX_VECTOR; // DMA1 transfer triggered by which interrupt? (On PIC32MX - it is by _IRQ suffix!)
  DCH1ECONbits.AIRQEN = 0; // do not enable DMA1 transfer abort interrupt.
  DCH1ECONbits.SIRQEN = 1; // enable DMA1 transfer start interrupt.

  // After the first half of the screen is transferred by DMA0, the other half follows at DMA1.
  setIntEnable(_DMA0_VECTOR);
  setIntVector(_DMA0_VECTOR, dma0isr);
  //setIntEnable(_DMA0_VECTOR);
  IPC33bits.DMA0IP = 7; // Highest priority for the master (DMA0)!
  IPC33bits.DMA0IS = 3;
  IPC33bits.DMA1IP = 7;
  IPC33bits.DMA1IS = 2; // Lower priority for this half of the screen because it's a channel chained slave (DMA1)!

  // Trip this interrupt so that the the system can draw the screen now.
  DCH1INTbits.CHBCIF = 1;
}

void loop() {
  // put your main code here, to run repeatedly:
  nes_main();
}
