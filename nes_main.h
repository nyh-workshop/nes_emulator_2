#ifndef _NES_MAIN_H_
#define _NES_MAIN_H_
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h> //memset();
#include "6502.h"
#include "PPU.h"
//#include "JoyPad.h"
#include "rom.h"
#include "APU.h"
/* define ------------------------------------------------------------------*/
#undef NULL
#define NULL 0
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif
#define _NES_DEBUG_
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    char filetype[4];      /* �ַ�����NES^Z������ʶ��.NES�ļ�        */
    unsigned char romnum;           /* 16kB ROM����Ŀ                         */
    unsigned char vromnum;          /* 8kB VROM����Ŀ                         */
    unsigned char romfeature;       /* D0��1����ֱ����0��ˮƽ����
                                       D1��1���е�ؼ��䣬SRAM��ַ$6000-$7FFF
                                       D2��1����$7000-$71FF��һ��512�ֽڵ�trainer
                                       D3��1��4��ĻVRAM����
                                       D4��D7��ROM Mapper�ĵ�4λ
                                    */
    unsigned char rommappernum;     /* D0��D3��������������0��׼����Ϊ��Mapper��^_^��
                                       D4��D7��ROM Mapper�ĸ�4λ*/                                    
} NesHeader;
/* function ------------------------------------------------------------------*/
void nes_main(void);
void NesFrameCycle(void);
void NES_ReadJoyPad(uint8 JoyPadNum);
#endif

