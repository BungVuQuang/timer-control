#include "flash.h"

void Flash_Unlock(void)
{
	/* This sequence consists of two write cycles, where two key values (KEY1 and KEY2) are written to the FLASH_KEYR address*/
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
}

FlashStatus Flash_Erase(volatile uint32_t u32StartAddr)
{
	while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Check unlock sequences */
	if ((FLASH->CR&FLASH_CR_LOCK) == FLASH_CR_LOCK )
	{
		Flash_Unlock();
	}
	/* Set the PER bit in the FLASH_CR register */
  FLASH->CR |= FLASH_CR_PER;
	/* Program the FLASH_AR register to select a page to erase */
  FLASH->AR = u32StartAddr;
	/* Set the STRT bit in the FLASH_CR register */
  FLASH->CR |= FLASH_CR_STRT;
	/* Wait for the BSY bit to be reset */
  while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Clear PER bit in the FLASH_CR register */
  FLASH->CR &= ~FLASH_CR_PER; 
	/* Clear STRT bit in the FLASH_CR register */
	FLASH->CR &= ~FLASH_CR_STRT;
	
	return FLASH_NO_ERRORS;
}

FlashStatus Flash_BackUp(volatile uint32_t u32StartAddr, uint8_t* u8BufferWrite)
{
	while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Check unlock sequences */
	if ((FLASH->CR&FLASH_CR_LOCK) == FLASH_CR_LOCK )
	{
		Flash_Unlock();
	}
	/* Write FLASH_CR_PG to 1 */
	FLASH->CR |= FLASH_CR_PG;
  /* Perform half-word write at the desired address*/
		*(uint16_t*)(u32StartAddr ) = *(uint16_t*)(u8BufferWrite);//ghi 2 byte 1

	/* Wait for the BSY bit to be reset */
  while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Clear PG bit in the FLASH_CR register */
	FLASH->CR &= ~FLASH_CR_PG;
	
	return FLASH_NO_ERRORS;
}

FlashStatus Flash_Write_Syn(volatile uint32_t u32StartAddr, uint8_t* u8BufferWrite, uint32_t u32Length)
{
	uint32_t u32Count = 0U;
	
	/* Check input paras */
	if((u8BufferWrite == 0x00U) || (u32Length == 0U) || u32Length%2U != 0U)//la so chan do phai viet 2 byte 1 lan`
	{
		return FLASH_ERRORS;
	}
   /* Check that no Flash memory operation is ongoing by checking the BSY bit in the FLASH_CR register */
	while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Check unlock sequences */
	if ((FLASH->CR&FLASH_CR_LOCK) == FLASH_CR_LOCK )
	{
		Flash_Unlock();
	}
	/* Write FLASH_CR_PG to 1 */
	FLASH->CR |= FLASH_CR_PG;
  /* Perform half-word write at the desired address*/
	for(u32Count = 0U; u32Count < (u32Length/2); u32Count ++ )
	{
		*(uint16_t*)(u32StartAddr + u32Count*2U) = *(uint16_t*)(u8BufferWrite + u32Count*2U);//ghi 2 byte 1
	}
	/* Wait for the BSY bit to be reset */
  while((FLASH->SR&FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		/*  Wating for Bsy bit */
	}
	/* Clear PG bit in the FLASH_CR register */
	FLASH->CR &= ~FLASH_CR_PG;
	
	return FLASH_NO_ERRORS;
}


FlashStatus Flash_Read(volatile uint32_t u32StartAddr, uint8_t* u8BufferRead, uint32_t u32Length)
{
	
	/* Check input paras */
	if((u8BufferRead == 0x00U) || (u32Length == 0U))
	{
		return FLASH_ERRORS;
	}
	do
	{
	   if(( u32StartAddr%4U == 0U) && (u32Length >= 4U))
		 {
		   *(uint32_t*)(u8BufferRead) = *(uint32_t*)(u32StartAddr);
		   u8BufferRead = u8BufferRead + 4U;
		 	 u32StartAddr = u32StartAddr + 4U;
			 u32Length = u32Length - 4U;
		 }
		 else if(( u32StartAddr%2U == 0U) && ((uint32_t)u8BufferRead%2U == 0U) && (u32Length >= 2U))
		 {
		    *(uint16_t*)(u8BufferRead) = *(uint16_t*)(u32StartAddr);
		 	  u8BufferRead = u8BufferRead + 2U;
		 	  u32StartAddr = u32StartAddr + 2U;
			  u32Length = u32Length - 2U;
		 }
		 else
		 {
		    *(uint8_t*)(u8BufferRead) = *(uint8_t*)(u32StartAddr);//doc 1 byte
		 	  u8BufferRead = u8BufferRead + 1U;
		 	  u32StartAddr = u32StartAddr + 1U;
			  u32Length = u32Length - 1U;
		 }
	}
	while(u32Length > 0U);
	
	return FLASH_NO_ERRORS;
}

