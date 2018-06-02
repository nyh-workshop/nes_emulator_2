// NES Classic Controller driver version 1.1.
// 2018 uncle-yong.
// 8/4/2018 - First release - v1.0.
// 2/6/2018 - v1.1: Added seperate I2C driver for Chipkit series.
//                  The Wire library for Chipkit is not working for this driver!

#include <Arduino.h>
#include "nesClassicController.h"
#include <Wire.h>
#include <chipKITVersion.h>

#if defined(__CHIPKIT__) <= 20004

void init_i2c() {
	CNPUDbits.CNPUD10 = 0;
	CNPUDbits.CNPUD9  = 0;
    TRISDbits.TRISD10 = 0;
	TRISDbits.TRISD9 = 0;
	I2C1STAT = 0x0000;
	
	I2C1CON = 0x000;
    I2C1CONbits.DISSLW = 1;
    I2C1CONbits.SMEN = 0;
    //I2C1ADD = 0b11010000;
    I2C1BRG = 111;   // 100kHz clock
    I2C1CONbits.A10M = 0;
    I2C1CONbits.ON = 1;
	
}

void i2c_wait() {
	while ( (I2C1CON & 0x1F) || I2C1STATbits.TRSTAT );

}

void i2c_start() {
    i2c_wait();
    I2C1CONbits.SEN = 1;
}

void i2c_restart() {
    i2c_wait();
    I2C1CONbits.RSEN = 1;
}

void i2c_stop() {
    i2c_wait();
    I2C1CONbits.PEN = 1;
}

void i2c_write(unsigned char data) {
    i2c_wait();
    I2C1TRN = data;
	while(I2C1STATbits.TBF);
}

void i2c_address(unsigned char address, unsigned char mode) {
    unsigned char l_address;

    l_address = address << 1;
    l_address |= mode;
    i2c_write(l_address);

}

unsigned char i2c_read(unsigned char ack) {

    unsigned char i2cReadData;

    i2c_wait();
    I2C1CONbits.RCEN = 1;
    i2c_wait();
    i2cReadData = I2C1RCV;
    i2c_wait();
    if (ack) I2C1CONbits.ACKDT = 0;
    else I2C1CONbits.ACKDT = 1;

    I2C1CONbits.ACKEN = 1;

    return i2cReadData;

}

#endif



void nesClassicController::init() {

  #if defined(__CHIPKIT__) <= 20004
  // For Chipkit only! (version 2.0.4 and below)	  
  // Init the controller first!
  init_i2c();
  delay(1);
  // Non-encrypted setting message:
  i2c_start();
  i2c_address(address, 0);
  i2c_write(0xF0);
  i2c_write(0x55);
  i2c_stop();
  
  i2c_start();
  i2c_address(address, 0);
  i2c_write(0xFB);
  i2c_write(0x00);
  i2c_stop();
  #else
   // Init the controller first!
  Wire.begin();
  // Non-encrypted setting message:
  Wire.beginTransmission(address);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();
  #endif  
  
}

void nesClassicController::readButtons() {
  
  #if defined(__CHIPKIT__) <= 20004
  // Send a read command: 	  
  i2c_start();
  i2c_address(address, 0);
  i2c_write(0x00);
  i2c_stop();
  
  i2c_start();
  i2c_address(address, 1);
  // Dump it into the i2c buffer:
  for (unsigned char i = 0; i < 7; i++) {
    i2cBuffer[i] = i2c_read(1);
  }
  i2cBuffer[7] = i2c_read(0);
  
  i2c_stop();
  #else
  // Send a read command:
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(address, 8);
  
  // Dump it into the i2c buffer:
  for (unsigned char i = 0; i < 8; i++) {
    i2cBuffer[i] = Wire.read();
  }  
  #endif
  
  // Take the last two bytes and merge them into the "tempButtons" variable, and invert it.
  tempButtons = ~( (i2cBuffer[6] << 8) | i2cBuffer[7] );

}

