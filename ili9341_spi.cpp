// ILI9341 (SPI) basic driver for Chipkit series.
// 2016-2018 uncle-yong.
// Reference: https://github.com/adafruit/Adafruit_ILI9341

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "ili9341_spi.h"
#include "font8x8.h"


#define SPIxFPB               99000000                    // 99MHz Peripheral Bus!
//#define SPIxFPB               49500000                  // 49.5MHz Peripheral Bus!
#define SPIxMIN_FREQ          SPIxFPB/(2*45000000)

uint16_t _width = 0;
uint16_t _height = 0;

volatile uint8_t __attribute__((coherent)) framebuffer[19200*2] = { 0 };      // 1/4 horizontal strip of the ILI9341 screen. (landscape orientation!)
volatile uint8_t __attribute__((coherent)) framebuffer2[64*2] = { 0 };      // for 8x8 font!
volatile uint8_t textBuffer40x25[1024] = { 0 }; // text buffer (40x25)!
volatile uint8_t textBufferAttr40x25[1024] = { 0 }; // text buffer (40x25) attributes!

const unsigned short BIOSColorAttr[16] = { 
    ILI9341_BLACK,
    ILI9341_BLUE,
    ILI9341_GREEN,
    ILI9341_CYAN,
    ILI9341_RED,
    ILI9341_MAGENTA,
    0xaaa0,               //ILI9341_BROWN
    ILI9341_LIGHTGREY,
    ILI9341_DARKGREY,
    0x52bf,               //ILI9341_LIGHTBLUE
    0x57ea,               //ILI9341_LIGHTGREEN
    0x57ff,               //ILI9341_LIGHTCYAN
    0xfaaa,               //ILI9341_LIGHTRED  
    0xfabf,               //ILI9341_LIGHTMAGENTA
    ILI9341_YELLOW,
    ILI9341_WHITE
};
       

void ILI9341_spi_begin() {
	
	// Todo: Add reconfigurable pins here (SPI1):
	// PLIB_PORTS_RemapOutput(PORTS_ID_0, OUTPUT_FUNC_SDO1, OUTPUT_PIN_RPD3 );
	// PLIB_PORTS_RemapInput(PORTS_ID_0, INPUT_FUNC_SDI1, INPUT_PIN_RPD2 );
	TRISDbits.TRISD2 = 1;
	TRISDbits.TRISD1 = 0;
	RPD3R = 0b0101;
	SDI1R = 0b0000;
	
    SPI1CONbits.ON = 0;
    SPI1CONbits.FRMEN = 0;
    SPI1CONbits.MCLKSEL = 0;
    SPI1CONbits.MODE32 = 0;
    SPI1CONbits.MODE16 = 0;
    SPI1CONbits.SSEN = 0;
    SPI1CONbits.MSTEN = 1;
    //SPI1CONbits.CKE = 1;
    //SPI1CONbits.CKP = 0;    // originally 0.
    SPI1CONbits.CKE = 1;
    SPI1CONbits.CKP = 1;
    SPI1CONbits.SMP = 0;
    SPI1CONbits.ENHBUF = 0;
    //SPI1CONbits.DISSDI = 1;
    
    SPI1STATCLR = (1<<6) | (1<<8);
   
    //SPI1BRG = SPIxMIN_FREQ;
    SPI1BRG = 0;
    
    // DMA Module here:
    DMACON  = 0x8000;                                            // enable DMA.
    DCH0CON = 0x0000;                                            // 
    DCRCCON = 0x00;                                              //   
    DCH0INT = 0x00;                                              // clear DMA interrupts register.
    //DCH0INTbits.CHBCIE = 1;                                      // DMA Interrupts when channel block transfer complete enabled.
    //DCH0INTbits.CHCCIE = 1;
    DCH0ECON = 0x00;
    //DCH0CONbits.CHPRI = 0b11; 
    DCH0SSA = (unsigned int)&framebuffer2 & 0x1FFFFFFF;           // DMA source address.
    DCH0DSA = (unsigned int)&SPI1BUF & 0x1FFFFFFF;               // DMA destination address.
    DCH0SSIZ = 64*2;                          // DMA Source size.
    DCH0DSIZ = 1;                            // DMA destination size.
    DCH0CSIZ = 1;                            // DMA cell size.
    DCH0ECONbits.CHSIRQ = _SPI1_TX_VECTOR;   // DMA transfer triggered by which interrupt?
    //DCH0ECONbits.CHSIRQ = _SPI1_RX_VECTOR;   // DMA transfer triggered by which interrupt?
    
    DCH0ECONbits.AIRQEN = 0;                 // do not enable DMA transfer abort interrupt.
    DCH0ECONbits.SIRQEN = 1;                 // enable DMA transfer start interrupt.
    
    DCH0CONbits.CHAEN = 0;                  // DMA Channel 0 is always enabled right after the transfer.   
    
    SPI1CONbits.ON = 1;
	
	
    
}

unsigned char ILI9341_spiwrite(unsigned char c) {
    //SPI1STATCLR = (1<<6);
    SPI1BUF = c;
	while (!SPI1STATbits.SPIRBF);  
    return SPI1BUF;
}

void ILI9341_writecommand(unsigned char c) {

    _dc = 0;
    _cs = 0;
    
    ILI9341_spiwrite(c);
    
    _cs = 1;
    
}

void ILI9341_writedata(unsigned char c) {
    _dc = 1;
    _cs = 0;
    
    ILI9341_spiwrite(c);
    
    _cs = 1;
} 

void ILI9341_begin(void) {
  //if (_rst > 0) {
    //pinMode(_rst, OUTPUT);
    //digitalWrite(_rst, LOW);
    //_rst_dir = 0;
    //_rst = 0;    
  //}

  //pinMode(_dc, OUTPUT);
  //pinMode(_cs, OUTPUT);
  _rst_dir = 0;
  _dc_dir = 0;
  _cs_dir = 0;
  
   // msb first
   // Mode 0
  ILI9341_spi_begin();


  // toggle RST low to reset
  //if (_rst > 0) {
    //digitalWrite(_rst, HIGH);
    _rst = 1;
    delay(5);
    //digitalWrite(_rst, LOW);
    _rst = 0;
    delay(20);
    //digitalWrite(_rst, HIGH);
    _rst = 1;
    delay(150);
  //}

  /*
  uint8_t x = readcommand8(ILI9341_RDMODE);
  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDMADCTL);
  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDPIXFMT);
  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDIMGFMT);
  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
  x = readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
*/
  //if(cmdList) commandList(cmdList);
   
  ILI9341_writecommand(0xEF);
  ILI9341_writedata(0x03);
  ILI9341_writedata(0x80);
  ILI9341_writedata(0x02);

  ILI9341_writecommand(0xCF);  
  ILI9341_writedata(0x00); 
  ILI9341_writedata(0XC1); 
  ILI9341_writedata(0X30); 

  ILI9341_writecommand(0xED);  
  ILI9341_writedata(0x64); 
  ILI9341_writedata(0x03); 
  ILI9341_writedata(0X12); 
  ILI9341_writedata(0X81); 
 
  ILI9341_writecommand(0xE8);  
  ILI9341_writedata(0x85); 
  ILI9341_writedata(0x00); 
  ILI9341_writedata(0x78); 

  ILI9341_writecommand(0xCB);  
  ILI9341_writedata(0x39); 
  ILI9341_writedata(0x2C); 
  ILI9341_writedata(0x00); 
  ILI9341_writedata(0x34); 
  ILI9341_writedata(0x02); 
 
  ILI9341_writecommand(0xF7);  
  ILI9341_writedata(0x20); 

  ILI9341_writecommand(0xEA);  
  ILI9341_writedata(0x00); 
  ILI9341_writedata(0x00); 
 
  ILI9341_writecommand(ILI9341_PWCTR1);    //Power control 
  ILI9341_writedata(0x23);   //VRH[5:0] 
 
  ILI9341_writecommand(ILI9341_PWCTR2);    //Power control 
  ILI9341_writedata(0x10);   //SAP[2:0];BT[3:0] 
 
  ILI9341_writecommand(ILI9341_VMCTR1);    //VCM control 
  ILI9341_writedata(0x3e); //�Աȶȵ���
  ILI9341_writedata(0x28); 
  
  ILI9341_writecommand(ILI9341_VMCTR2);    //VCM control2 
  ILI9341_writedata(0x86);  //--
 
  ILI9341_writecommand(ILI9341_MADCTL);    // Memory Access Control 
  //ILI9341_writedata(0x48);
  ILI9341_writedata(0xE8);

  
  ILI9341_writecommand(ILI9341_PIXFMT);    
  ILI9341_writedata(0x55); 
  
  ILI9341_writecommand(ILI9341_FRMCTR1);    
  ILI9341_writedata(0x00);  
  //ILI9341_writedata(0x18);
  ILI9341_writedata(0x1f); 
 
 
  ILI9341_writecommand(ILI9341_DFUNCTR);    // Display Function Control 
  ILI9341_writedata(0x08); 
  ILI9341_writedata(0x82);
  ILI9341_writedata(0x27);  
 
  /*
  ILI9341_writecommand(0xF2);    // 3Gamma Function Disable 
  ILI9341_writedata(0x00); 
 
  ILI9341_writecommand(ILI9341_GAMMASET);    //Gamma curve selected 
  ILI9341_writedata(0x01); 
 
  ILI9341_writecommand(ILI9341_GMCTRP1);    //Set Gamma 
  ILI9341_writedata(0x0F); 
  ILI9341_writedata(0x31); 
  ILI9341_writedata(0x2B); 
  ILI9341_writedata(0x0C); 
  ILI9341_writedata(0x0E); 
  ILI9341_writedata(0x08); 
  ILI9341_writedata(0x4E); 
  ILI9341_writedata(0xF1); 
  ILI9341_writedata(0x37); 
  ILI9341_writedata(0x07); 
  ILI9341_writedata(0x10); 
  ILI9341_writedata(0x03); 
  ILI9341_writedata(0x0E); 
  ILI9341_writedata(0x09); 
  ILI9341_writedata(0x00); 
  
  ILI9341_writecommand(ILI9341_GMCTRN1);    //Set Gamma 
  ILI9341_writedata(0x00); 
  ILI9341_writedata(0x0E); 
  ILI9341_writedata(0x14); 
  ILI9341_writedata(0x03); 
  ILI9341_writedata(0x11); 
  ILI9341_writedata(0x07); 
  ILI9341_writedata(0x31); 
  ILI9341_writedata(0xC1); 
  ILI9341_writedata(0x48); 
  ILI9341_writedata(0x08); 
  ILI9341_writedata(0x0F); 
  ILI9341_writedata(0x0C); 
  ILI9341_writedata(0x31); 
  ILI9341_writedata(0x36); 
  ILI9341_writedata(0x0F); 
  */

  ILI9341_writecommand(ILI9341_SLPOUT);    //Exit Sleep 
  //if (hwSPI) spi_end();
  delay(200); 		
  //if (hwSPI) spi_begin();
  ILI9341_writecommand(ILI9341_DISPON);    //Display on 
  //if (hwSPI) spi_end();

}

void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) {

  ILI9341_writecommand(ILI9341_CASET); // Column addr set
  ILI9341_writedata(x0 >> 8);
  ILI9341_writedata(x0 & 0xFF);     // XSTART 
  ILI9341_writedata(x1 >> 8);
  ILI9341_writedata(x1 & 0xFF);     // XEND

  ILI9341_writecommand(ILI9341_PASET); // Row addr set
  ILI9341_writedata(y0>>8);
  ILI9341_writedata(y0);     // YSTART
  ILI9341_writedata(y1>>8);
  ILI9341_writedata(y1);     // YEND

  ILI9341_writecommand(ILI9341_RAMWR); // write to RAM
}

void ILI9341_drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  ILI9341_setAddrWindow(x,y,x+1,y+1);

  //digitalWrite(_dc, HIGH);
  //digitalWrite(_cs, LOW);
  _dc = 1;
  _cs = 0;

  ILI9341_spiwrite(color >> 8);
  ILI9341_spiwrite(color);

  //digitalWrite(_cs, HIGH);

  _cs = 1;
  
}

void ILI9341_blit(int16_t x, int16_t y, int16_t imgSize_x, int16_t imgSize_y, uint16_t* inputImg) {
       
    unsigned int i = 0;
    
    for (i = 0; i < 1024 ; i++)
        framebuffer[i] = inputImg[i];
    
    ILI9341_setAddrWindow(x,y,x+imgSize_x,y+imgSize_y);
    
    _dc = 1;
    _cs = 0;
    
    
    DCH0INT = 0x00;
    DCH0CONbits.CHEN = 1;                   
    DCH0ECONbits.CFORCE = 1;
    while(DCH0INT != 0x0b);
    //while(!DCH0INTbits.CHBCIF);
    //while(!DCH0INTbits.CHCCIF);
    //DCH0INTbits.CHBCIF = 0;
    
    DCH0CONbits.CHEN = 0; 
        
    _cs = 1;
        
}

const unsigned char chr8x16[] = { 0xc9,  0x01,  0x4d,  0x02,  0x08,  0xe0,  0x41,  0x1e,  0x00,  0x54,  0x2a,  0x04,  0x00,  0x12,  0x81,  0x18 };

void ILI9341_blit_chr_8x8(int16_t x, int16_t y, int16_t b_color, int16_t f_color, uint8_t inputChr) {
    
    //DCH0INT = 0x00;
    DCH0SSA = (unsigned int)&framebuffer2 & 0x1FFFFFFF;           // DMA source address.
    DCH0DSA = (unsigned int)&SPI1BUF & 0x1FFFFFFF;               // DMA destination address.
    DCH0SSIZ = 64*2;                          // DMA Source size.
    DCH0DSIZ = 1;                            // DMA destination size.
    DCH0CSIZ = 1;                            // DMA cell size.
    
//    mode16:
//    DCH0SSIZ = 64*2;                          // DMA Source size.
//    DCH0DSIZ = 2;                            // DMA destination size.
//    DCH0CSIZ = 2;                            // DMA cell size.
        
    unsigned int i = 0, j = 0;
    
    
    
    //
    /*
    for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                if (((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01)) {
                    framebuffer2[(2 * j)+(i * 16)] = ((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01) * (color >> 8);
                    framebuffer2[(2 * j + 1)+(i * 16)] = ((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01) * (color & 0x0f);
                } else {
                    framebuffer2[(2 * j)+(i * 16)] = ((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01) * (color >> 8);
                    framebuffer2[(2 * j + 1)+(i * 16)] = ((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01) * (color & 0x0f);
                }
            }
        } */  
    
    for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                if (((font8x8[(inputChr * 8) + i] >> (7 - j)) & 0x01)) {
                    framebuffer2[(2 * j)+(i * 16)]     = (f_color >> 8);
                    framebuffer2[(2 * j + 1)+(i * 16)] = (f_color & 0x0f);
                } else {
                    framebuffer2[(2 * j)+(i * 16)]     =  (b_color >> 8);
                    framebuffer2[(2 * j + 1)+(i * 16)] =  (b_color & 0x0f);
                }
            }
        }
    
    ILI9341_setAddrWindow(x,y,x+7,y+7);
       
    _dc = 1;
    _cs = 0;    
    
    DCH0CONbits.CHEN = 1;   
    DCH0ECONbits.CFORCE = 1;
    while(!DCH0INTbits.CHBCIF);
    //while(DCH0INT != 0x0b);
    DCH0INTbits.CHBCIF = 0;
    //while(!DCH0INTbits.CHCCIF);
    
       
    DCH0CONbits.CHEN = 0;
    _cs = 1;
    
    // Is there a reason why I should restart the SPI after a DMA transfer??
    // Receive Overflow bit in SPIxSTAT is up - clearing it doesn't work at all on this model.
    // Only way to clear it is to restart SPI module.
    //SPI1CONbits.ON = 0;
    //SPI1CONbits.ON = 1;
	SPI1CONCLR = 1<<15;
	SPI1CONSET = 1<<15;
    SPI1STATCLR = (1<<6);
    
    // For non-DMA transfers: 
    /*
    for(i = 0; i < 64*2; i++) {
        ILI9341_writedata(framebuffer2[i]);
    }*/
    
}

void ILI9341_write_chr_40x25(unsigned char col, unsigned char row, unsigned char b_color, unsigned char f_color, unsigned char inputChr) {
    
    
    textBuffer40x25[col + row*40]     = inputChr;
    textBufferAttr40x25[col + row*40] = ((b_color & 0x0f) << 4) | (f_color & 0x0f);
      
}

void ILI9341_write_txt_40x25(unsigned char col, unsigned char row, unsigned char b_color, unsigned char f_color, unsigned char* inputTxt) {
    
    unsigned int length = 0;
    while (*inputTxt) {
        textBuffer40x25[(col + row*40)+length] = *inputTxt++;
        textBufferAttr40x25[(col + row*40)+length] = ((b_color & 0x0f) << 4) | (f_color & 0x0f);
        length++;
    }   
}

void ILI9341_write_screen_40x25() {
    
    unsigned char row = 0, col = 0;
    for(row = 0; row < 25; row++)
        for(col = 0; col < 40; col++)
            ILI9341_blit_chr_8x8(8*col, 8*row, BIOSColorAttr[textBufferAttr40x25[col+40*row] >> 0x04], BIOSColorAttr[textBufferAttr40x25[col+40*row] & 0x0f], textBuffer40x25[col+40*row]);
    
}

void ILI9341_clrScreen(void) {
    
    unsigned int i = 0;
    DCH0SSA = (unsigned int)&framebuffer & 0x1FFFFFFF;           // DMA source address.
    DCH0DSA = (unsigned int)&SPI1BUF & 0x1FFFFFFF;               // DMA destination address.
    DCH0SSIZ = 19200*2;                      // DMA Source size.
    DCH0DSIZ = 1;                            // DMA destination size.
    DCH0CSIZ = 1;                            // DMA cell size.
    
    memset((void*)framebuffer, 0x00, 19200*2);
    
    ILI9341_setAddrWindow(0,0,320-1,240-1);
    
    _dc = 1;
    _cs = 0;
    
    for (i = 0; i < 4; i++) {
        DCH0CONbits.CHEN = 1;
        DCH0ECONbits.CFORCE = 1;
        while (!DCH0INTbits.CHBCIF);
        DCH0INTbits.CHBCIF = 0;
		SPI1CONCLR = 1<<15;
		SPI1CONSET = 1<<15;
		SPI1STATCLR = (1<<6);
    }
    _cs = 1;
    
    SPI1CONbits.ON = 0;
    SPI1CONbits.ON = 1;
    SPI1STATCLR = (1<<6);
}

void ILI9341_setRotation(uint8_t m) {

  unsigned char rotation = 0;
  //if (hwSPI) spi_begin();
  ILI9341_writecommand(ILI9341_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     ILI9341_writedata(MADCTL_MX | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
     break;
   case 1:
     ILI9341_writedata(MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  case 2:
    ILI9341_writedata(MADCTL_MY | MADCTL_BGR);
     _width  = ILI9341_TFTWIDTH;
     _height = ILI9341_TFTHEIGHT;
    break;
   case 3:
     ILI9341_writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
     _width  = ILI9341_TFTHEIGHT;
     _height = ILI9341_TFTWIDTH;
     break;
  }
  //if (hwSPI) spi_end();
}
