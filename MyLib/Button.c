#include "Button.h"
//---------------var button------------


__weak void	btn_pressing_callback(Button_Typdef *ButtonX)
{
}
__weak void btn_press_short_callback(Button_Typdef *ButtonX )
{
	
}
__weak void btn_release_callback(Button_Typdef *ButtonX)
{

}
__weak void btn_press_timeout_callback(Button_Typdef *ButtonX)
{

}

__weak void btn_press_timeout_off_callback(Button_Typdef *ButtonX)
{

}

void button_handle(Button_Typdef *ButtonX)
{
	//------------------ Loc nhieu ------------------------
	uint8_t sta = HAL_GPIO_ReadPin(ButtonX->GPIOx, ButtonX->GPIO_Pin);
	if(sta != ButtonX->btn_filter)
	{
		ButtonX->btn_filter = sta;
		ButtonX->is_debouncing = 1;
		ButtonX->time_deboune = HAL_GetTick();
	}
	//------------------ Tin hieu da xac lap------------------------
	if(ButtonX->is_debouncing && (HAL_GetTick() - ButtonX->time_deboune >= 15))
	{
		ButtonX->btn_current = ButtonX->btn_filter;
		ButtonX->is_debouncing =0;
	}
	//---------------------Xu li nhan nha------------------------
	if(ButtonX->btn_current != ButtonX->btn_last)
	{
		if(ButtonX->btn_current == 0)//nhan xuong
		{
			ButtonX->is_press_timeout = 1;
			ButtonX->is_press_timeout_off = 1;
			btn_pressing_callback(ButtonX);
			ButtonX->time_start_press = HAL_GetTick();
		}
		else //nha nut
		{
			if(HAL_GetTick() - ButtonX->time_start_press <= 1000)
			{
				btn_press_short_callback(ButtonX);
			}
			btn_release_callback(ButtonX);
			ButtonX->is_press_timeout = 0;
			ButtonX->is_press_timeout_off = 0;
		}
		ButtonX->btn_last = ButtonX->btn_current;
	}
	//-------------Xu li nhan giu----------------
	if(ButtonX->is_press_timeout && (HAL_GetTick() - ButtonX->time_start_press >= 2000) && (HAL_GetTick() - ButtonX->time_start_press <= 4000))
	{
		btn_press_timeout_callback(ButtonX);
		ButtonX->is_press_timeout =0;
	}
		if(ButtonX->is_press_timeout_off && (HAL_GetTick() - ButtonX->time_start_press > 4000))
	{
		btn_press_timeout_off_callback(ButtonX);
		ButtonX->is_press_timeout =0;
		ButtonX->is_press_timeout_off =0;
	}
}

void Interrupt_Config (void)
{
	RCC->APB2ENR |= (1<<0);  // Enable AFIO CLOCK
	
	AFIO->EXTICR[0] &= ~(0xf<<16);  // Bits[19:18:17:16] = (0:0:0:0)  -> configure EXTI1 line for PA4
	AFIO->EXTICR[0] &= ~(0xf<<20);  
	AFIO->EXTICR[0] &= ~(0xf<<24);  
	AFIO->EXTICR[0] &= ~(0xf<<0);  
	EXTI->IMR |= (1<<0)|(1<<4)|(1<<5) |(1<<6);  // Bit[1] = 1  --> Disable the Mask on EXTI
	EXTI->RTSR |= (1<<0)|(1<<4)|(1<<5)|(1<<6);  // Enable Rising Edge Trigger for PA
	
	EXTI->FTSR &= ~((1<<0)|(1<<4)|(1<<5)|(1<<6));  // Disable Falling Edge Trigger for PA
	
	NVIC_SetPriority (EXTI4_IRQn, 1);  // Set Priority
	NVIC_SetPriority (EXTI9_5_IRQn, 1);  // Set Priority
	NVIC_SetPriority (EXTI0_IRQn, 1);  // Set Priority
	NVIC_EnableIRQ (EXTI4_IRQn);  // Enable Interrupt
	NVIC_EnableIRQ (EXTI0_IRQn);  // Enable Interrupt
	NVIC_EnableIRQ (EXTI9_5_IRQn);  // Enable Interrupt
	
}

void button_init(Button_Typdef *ButtonX,GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	ButtonX->btn_filter=1;
	ButtonX->btn_last=1;
	ButtonX->btn_current=1;
	ButtonX->GPIOx = GPIOx;
	ButtonX->GPIO_Pin = GPIO_Pin;
}
