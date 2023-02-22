#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include "stm32f1xx_hal.h"
#include "Delay.h"
#include <stdint.h>

#define I2C_LCD_ADDR 0x4E

#define I2C_LCD_Delay_Ms(u16DelayMs) Delay_Ms(u16DelayMs)
//khai bao cac chan nhu trong schmatic
#define LCD_EN 2//PB2
#define LCD_RW 1//PB1
#define LCD_RS 0//PB0
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define LCD_BL 3//u8BackLight

void I2C_LCD_Init(void);
void I2C_LCD_Puts(char *szStr, ...);
void I2C_LCD_Clear(void);
void I2C_LCD_NewLine(void);
void I2C_LCD_BackLight(uint8_t u8BackLight);
void I2C_LCD_GotoXY(unsigned char x, unsigned char y);
#endif
