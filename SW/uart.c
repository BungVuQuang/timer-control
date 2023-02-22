#include "uart.h"
#include "ring_buffer.h"

#define MAX_UART_LEN	128
static UART_HandleTypeDef *uart;
uint8_t buff_uart[MAX_UART_LEN];
circ_bbuf_t ring_buff;
uint8_t uart_rx;

void uart_receive_rx(uint8_t data_rx){
	circ_bbuf_push(&ring_buff, data_rx);
}

uint16_t uart_available(void){
	return ring_buff_available(&ring_buff);
}
uint8_t uart_read(void)
{
	uint8_t data;
	circ_bbuf_pop(&ring_buff,&data);
	return data;
}
void uart_init(UART_HandleTypeDef *huart1)
{
	circ_bbuf_init(&ring_buff, buff_uart, MAX_UART_LEN);
	uart = huart1;
	//HAL_UART_Receive_IT(uart, &uart_rx, 1);
}
