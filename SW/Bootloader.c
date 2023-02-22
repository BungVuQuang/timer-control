#include "uart.h"
#include "Bootloader.h"
#include "min.h"
#include "Bootloader_Command.h"
#include "response_cli.h"
#include "flash.h"
#include "sys_config.h"
#include "utils.h"

//#define MIN_PORT 0
//#define MIN_ID 1

//uint8_t cnt_flash = 0;
//uint8_t page_flash = 0;


//struct min_context min_ctx;


//void bootloader_send_data(void *data, uint8_t len)
//{
//    min_send_frame(&min_ctx, MIN_ID, (uint8_t *)data, len);
//}

//void bootloader_handler(void){
//	uint8_t data;
//	uint8_t len =0;
//	if(uart_available()){	
//		data = uart_read();
//		min_poll(&min_ctx, &data,1);//dua du lieu vao xem co hop le hay k
//		//len = 1;
//	}
//}

//void bootloader_init(void){
//	min_init_context(&min_ctx, MIN_PORT);
//}	
