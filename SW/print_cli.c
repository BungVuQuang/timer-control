#include "print_cli.h"
#include "stdarg.h"
#include "main.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;
//print_cli("hello %d %f",12,12.4);
void print_cli(const char* str, ...)
{
	char stringArray[128];
	
  va_list args;
  va_start(args, str);
  uint8_t len_str = vsprintf(stringArray, str, args);
  va_end(args);
	
	HAL_UART_Transmit(&huart1,(uint8_t*)stringArray, len_str, 100);
}
