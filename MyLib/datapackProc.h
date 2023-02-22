#ifndef HEADER_GUARD
#define HEADER_GUARD
/*-----------------------------------------------------------------------------*/
/* Header inclusions */
/*-----------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "main.h"
/*-----------------------------------------------------------------------------*/
/* Constant definitions  */
/*-----------------------------------------------------------------------------*/
#define MAX_LENGTH 100
#define START_BYTE 0x55  // 01010101
#define STOP_BYTE 0xAA   // 10101010
#define REPEAT_BYTE 0x5A // 10101010
#define ESC_BYTE 0x7D    // 1111101

// Choose new indicator to test by terminal
// #define START_BYTE 't'
// #define STOP_BYTE 's'
// #define ESC_BYTE 'e'

/*-----------------------------------------------------------------------------*/
/* Data type definitions */
/*-----------------------------------------------------------------------------*/

// Data Package data struct
typedef struct tagDATAPACKAGE
{
    uint8_t data[MAX_LENGTH];
    uint32_t crc;
    uint32_t length;
} __attribute__((packed)) DATAPACKAGE;

// State of receive data
typedef enum
{
    WAIT_START_BYTE,
    IN_MESG,
    STOP,
} RX_STATE;

typedef enum
{
    DATA_OK,
    DATA_ERROR,
} BOOL_INVALID;

typedef enum
{
    REPEAT,
    NOT_REPEAT,
} REPEAT_BOOL;

/*-----------------------------------------------------------------------------*/
/* Macro definitions  */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Global variables  */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function prototypes  */
/*-----------------------------------------------------------------------------*/
uint32_t crc32(uint8_t *data, uint16_t length);
void InitRxDataPackage();
void CollectingDataPackage(void *buff);
BOOL_INVALID IsInvalidDataPackage(DATAPACKAGE *mesg);
void EncodeDatPackage(void *Buff, DATAPACKAGE *pMesg, REPEAT_BOOL repeat);
void DecodeDataPackage(DATAPACKAGE *mesg, void *resul);

#endif // HEADER_GUARD