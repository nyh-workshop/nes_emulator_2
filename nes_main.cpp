#include "nes_main.h"
#include "Arduino.h"
#include "sys/kmem.h"
#include "ili9341_spi.h"
#include "nesClassicController.h"
//#include "APU.h"

// Frame buffer routines and extern here:
inline void draw_frame(void) ;
extern uint16 __attribute__((coherent)) frameBuffer[256 * 240];

// NES classic controller here:
nesClassicController ncc;

uint8 Continue = TRUE;//��ʼ��Ϊ��
int FrameCnt;
//-------------------------------------------------------------------------------
/* NES ֡����ѭ��*/
void NesFrameCycle(void)
{
  int clocks; //CPUִ��ʱ��
  FrameCnt = 0;
  while (Continue)
  {
    /* scanline: 0~19 VBANK �Σ���PPUʹ��NMI��������NMI �ж�, */
    FrameCnt++;        //֡������
    SpriteHitFlag = FALSE;
    for (PPU_scanline = 0; PPU_scanline < 20; PPU_scanline++)
    {
      exec6502(CLOCKS_PER_SCANLINE);
    }

    /* scanline: 20, PPU�������ã�ִ��һ�οյ�ɨ��ʱ��*/
    exec6502(CLOCKS_PER_SCANLINE);
    PPU_scanline++;   //20++
    PPU_Reg.R2 &= ~R2_SPR0_HIT;
    /* scanline: 21~261*/
    for (; PPU_scanline < SCAN_LINE_DISPALY_END_NUM; PPU_scanline++)
    {
      if ((SpriteHitFlag == TRUE) && ((PPU_Reg.R2 & R2_SPR0_HIT) == 0))
      {
        clocks = sprite[0].x * CLOCKS_PER_SCANLINE / NES_DISP_WIDTH;
        exec6502(clocks);
        PPU_Reg.R2 |= R2_SPR0_HIT;
        exec6502(CLOCKS_PER_SCANLINE - clocks);
      }
      else
      {
        exec6502(CLOCKS_PER_SCANLINE);  // tlb exception occurs @ FrameCnt = 8 and PPU_scanline = 38
      }
      if (PPU_Reg.R1 & (R1_BG_VISIBLE | R1_SPR_VISIBLE)) //��Ϊ�٣��ر���ʾ
      {
        if (SpriteHitFlag == FALSE)
        {
          NES_GetSpr0HitFlag(PPU_scanline - SCAN_LINE_DISPALY_START_NUM);//����Sprite #0 ��ײ��־
        }
      }
      if (FrameCnt & 2) //ÿ2֡��ʾһ��
      {
        NES_RenderLine(PPU_scanline - SCAN_LINE_DISPALY_START_NUM);//ˮƽͬ������ʾһ��
      }
       //Added APU routines:
       switch (PPU_scanline) {
        
        case SCAN_APU_CLK1: // 240Hz
        case SCAN_APU_CLK3: // 240Hz
          pNesX_ApuClk_240Hz();
          break;

        case SCAN_APU_CLK2: // 240Hz 120Hz
          pNesX_ApuClk_240Hz();
          pNesX_ApuClk_120Hz();
          break;

        case SCAN_APU_CLK4: // 240Hz 120Hz 60Hz
          pNesX_ApuClk_240Hz();
          pNesX_ApuClk_120Hz();
          pNesX_ApuClk_60Hz();
          break;

       default: break;           
     }
       
   }

    /* scanline: 262 ���һ֡*/

    exec6502(CLOCKS_PER_SCANLINE);
    PPU_Reg.R2 |= R2_VBlank_Flag;   //����VBANK ��־

    /*��ʹ��PPU VBANK�жϣ�������VBANK*/
    if (PPU_Reg.R0 & R0_VB_NMI_EN)
    {
      NMI_Flag = SET1;    //���һ֡ɨ�裬����NMI�ж�
    }
    draw_frame();

  }
}
//-------------------------------------------------------------------------------
void nes_main(void)
{
  ApuInit();
  
  ncc.init();
  
  NesHeader *neshreader = (NesHeader *) rom_file;
  init6502mem( 0,         /*exp_rom*/
               0,         /*sram �ɿ����;���, �ݲ�֧��*/
               (&rom_file[0x10]),      /*prg_rombank, �洢����С �ɿ����;���*/
               neshreader->romnum
             );  //��ʼ��6502�洢������

  reset6502();
  
  PPU_Init((&rom_file[0x10] + (neshreader->romnum * 0x4000)), (neshreader->romfeature & 0x01));  /*PPU_��ʼ��*/

  Serial.println("finished ppu init");
  NesFrameCycle();
  
}
//-------------------------------------------------------------------------------


inline void draw_frame(void) {
  //while (!DCH0INTbits.CHBCIF);
  //
  //      DCH0INTbits.CHBCIF = 0;
  //      DCH0INT = 0x0000;
  //      SPI1STAT = 0x0000;
  //
  //
  //      SPI1CONbits.ON = 0;
  //      SPI1CONbits.ON = 1;
  //
  //      //SPI1CONbits.DISSDI = 0;
  //
  //      ILI9341_setAddrWindow(0, 0, 256, 240);
  //
  //      _dc = 1;
  //      _cs = 0;
  //
  //      // PORTDbits.RD4 = 1;
  //      DCH0INTbits.CHBCIF = 0;
  //      DCH0INT = 0x0000;
  //      SPI1STAT = 0x0000;
  //
  //      SPI1CONbits.ON = 0;
  //      SPI1CONbits.ON = 1;
  //
  //      DCH0CONbits.CHEN = 0;
  //
  //      DCH0DSA = KVA_TO_PA(&SPI1BUF);
  //      DCH0SSA = KVA_TO_PA(&frameBuffer[0]);
  //      DCH0SSIZ = 256 * 240 * 2;                        // DMA Source size.
  //      DCH0DSIZ = 1;                            // DMA destination size.
  //      DCH0CSIZ = 1;                            // DMA cell size.
  //
  //      //SPI1CONbits.DISSDI = 1;
  //      DCH0CONbits.CHEN = 1;
  //      DCH0ECONbits.CFORCE = 1;
  // bottom half of screen!


  while (!DCH1INTbits.CHBCIF);
  DCH0INTCLR = 0x00ff;
  DCH1INTCLR = 0x00ff;
  //SPI1STAT = 0x0000;

  DCH1CONbits.CHEN = 0;
  DCH0CONbits.CHEN = 0;
  SPI1CONbits.DISSDI = 0;
  DMACONbits.ON = 0;

  // Restart the SPI module! Manually resetting the SPI flags doesn't work!
  SPI1CONbits.ON = 0;
  SPI1CONbits.ON = 1;

  // Realign the address window back to the 256x240.
  ILI9341_setAddrWindow(0, 0, 256, 240);

  SPI1CONbits.DISSDI = 1;

  // Restart the SPI module again!
  SPI1CONbits.ON = 0;
  SPI1CONbits.ON = 1;

  _cs = 1;

  // One half of the screen is taking 61,440 bytes! The PIC32's DMA only supports up to 64KB of it!
  DCH0SSIZ = 256 * 240;                     // DMA Source size.
  DCH0DSIZ = 1;                            // DMA destination size.
  DCH0CSIZ = 1;                            // DMA cell size.

  DCH0DSA = KVA_TO_PA(&SPI1BUF);
  DCH0SSA = KVA_TO_PA(&frameBuffer[0]);

  DCH1SSIZ = 256 * 240;                     // DMA Source size.
  DCH1DSIZ = 1;                            // DMA destination size.
  DCH1CSIZ = 1;                            // DMA cell size.

  DCH1DSA = KVA_TO_PA(&SPI1BUF);
  DCH1SSA = KVA_TO_PA(&frameBuffer[0] + 30720);

  _dc = 1;
  _cs = 0;

  // Start transferring the data to the top half of screen first!
  DMACONbits.ON = 1;
  DCH0INTbits.CHBCIE = 1; // enable block transfer complete interrupt!
  DCH0CONbits.CHEN = 1;
  DCH0ECONbits.CFORCE = 1;
}

unsigned char NES_GetJoyPadVlaue(unsigned char JOYPAD_0) {
	
	
	
}

