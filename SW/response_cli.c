#include "response_cli.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *huart_print;

void response_print(const char* str, ...)
{
	char stringArray[128];
	
  va_list args;
  va_start(args, str);
  uint8_t len_str = vsprintf(stringArray, str, args);
  va_end(args);
	
	HAL_UART_Transmit(huart_print,(uint8_t*)stringArray, len_str, 100);
}

void response_cli_init(UART_HandleTypeDef *_huart_print)
{
	huart_print = _huart_print;
}
