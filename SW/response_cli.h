#ifndef RESPONE_CLI_H
#define RESPONE_CLI_H
#include "main.h"
void response_print(const char* str, ...);
void response_cli_init(UART_HandleTypeDef *_huart_print);
#endif
