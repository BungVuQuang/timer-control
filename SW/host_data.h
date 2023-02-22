
#ifndef __HOST_DATA1_H
#define __HOST_DATA1_H

#include "min.h"
#include "uart.h"
#include "alarm_command.h"
#include "ring_buffer.h"
#include "response_cli.h"

#define  MIN_PORT 0
#define  MIN_ID 1

typedef enum
{
    CLOCK_IDLE_STATE,
    CLOCK_START_STATE,
    CLOCK_STM32_SEND_STATE,
    CLOCK_SETTING_STATE,
    CLOCK_ALARM_STATE,
    CLOCK_SEND_DATA_STATE,
    CLOCK_END_STATE
} Clock_State_Frame_t;

void clock_send_command(ClockCommnadType clock_cmd_type);

void clock_sent_start_signal(void);

void clock_send_finish(ClockCommnadType clock_cmd_type);

void clock_send_response(ClockResponse clock_response_type);

void min_send_data(void *data, uint8_t len);

void clock_send_data(uint8_t *data, uint8_t len);

void clock_min_handler(void);

void clock_min_init(void);

#endif
