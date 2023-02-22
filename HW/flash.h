#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"
#define FALSE 0U
#define TRUE 1U
#define LENGBUFFER 8U

typedef enum
{
	FLASH_ERRORS = 0U,    /* There is a error */
	FLASH_NO_ERRORS,      /* There is no errors */
	FLASH_PENDING,        /* Working is pending  */
	FLASH_ERRORS_TIMEOUT  /* There is a error due to timeout */
} FlashStatus;
/**********************************************************************************************************************
*                                              FLASH
***********************************************************************************************************************/
void Flash_Unlock(void);
FlashStatus Flash_Erase(volatile uint32_t u32StartAddr);
FlashStatus Flash_Write_Syn(volatile uint32_t u32StartAddr, uint8_t* u8BufferWrite, uint32_t u32Length);
FlashStatus Flash_BackUp(volatile uint32_t u32StartAddr, uint8_t* u8BufferWrite);
FlashStatus Flash_Read(volatile uint32_t u32StartAddr, uint8_t* u8BufferRead, uint32_t u32Length);
#endif
