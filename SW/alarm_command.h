#ifndef __ALARM_COMMAND_H
#define __ALARM_COMMAND_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef enum
{
    CLOCK_COMMAND,
    CLOCK_DATA,
    CLOCK_FINISH
}ClockCommandId;

typedef enum
{
    START_COMMAND,
    STOP_COMMAND,
    SETTING_COMMAND,
    ALARM_COMMAND,
} ClockCommnadType;

typedef enum
{
    ACK,
    NACK
} ClockResponse;

typedef struct
{
    uint8_t command_id;
    uint8_t len;
    uint8_t clock_cmd;
} __attribute__((packed)) ClockCommand;

typedef struct
{
    uint8_t command_id;
    uint8_t len;
    uint8_t ack;
} __attribute__((packed)) ClockResponseFrame;

typedef struct
{
    uint8_t command_id;
    uint8_t len;
    uint8_t data[50];
} __attribute__((packed)) ClockData;
#endif
