#ifndef UART_H
#define UART_H
#include "main.h"
void uart_init(UART_HandleTypeDef *huart1);
uint8_t uart_read(void);
void uart_receive_rx(uint8_t data_rx);
uint16_t uart_available(void);
#endif
