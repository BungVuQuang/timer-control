#include "datapackProc.h"
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/
volatile uint8_t g_flagFrameReceived = 0;
DATAPACKAGE DataPackageRX;
RX_STATE RX_state; // state of rx data.
static volatile uint8_t lastRxData;
static volatile uint8_t nIndex;
/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Function implementations */
/*-----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------

Function: InitRxDataPackage
Purpose: Init Rx Data Package
Parameters: RX_STATE state
Return: NULL
--------------------------------------------------------------------------------*/
void InitRxDataPackage()
{
    // RegisterUartCallback(UART_DATA_EVENT, CollectingDataPackage);
}

uint32_t crc32(uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint32_t crc = 0xffffffff; // Initial value
    while (length--)
    {
        crc ^= *data++; // crc ^= *data; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320; // 0xEDB88320= reverse 0x04C11DB7
            else
                crc = (crc >> 1);
        }
    }
    return ~crc;
}

/*--------------------------------------------------------------------------------
Function: CollectingDataPackage
Purpose: Collecting Data package and store in a buffer
Parameters: void* buff
Return: NULL
--------------------------------------------------------------------------------*/

void CollectingDataPackage(void *rxBuff)
{
    uint8_t *pRxBuff = (uint8_t *)rxBuff;
    switch (RX_state)
    {
    case WAIT_START_BYTE:
    {
        lastRxData = '\0';
        nIndex = 0;
        if (*pRxBuff == START_BYTE)
        {
            DataPackageRX.data[nIndex++] = START_BYTE;
            lastRxData = *pRxBuff;
            RX_state = IN_MESG;
        }
        break;
    }
    case IN_MESG:
    {
        if (*pRxBuff == STOP_BYTE && lastRxData != ESC_BYTE)
        {
            RX_state = STOP;
            break;
        }
        if (*pRxBuff == START_BYTE && lastRxData != ESC_BYTE)
        {
            // Restart frame data.
            nIndex = 0;
            DataPackageRX.data[nIndex++] = *pRxBuff;
            lastRxData = *pRxBuff;
            break;
        }

        DataPackageRX.data[nIndex++] = *pRxBuff;
        lastRxData = *pRxBuff;
        break;
    }
    case STOP:
    {
        DataPackageRX.data[nIndex++] = STOP_BYTE;
        DataPackageRX.data[nIndex] = '\0';
        DataPackageRX.length = nIndex;
        // WriteUart(DataPackageRX.data, nIndex);

        // Set flag to process frame data
        g_flagFrameReceived = 1;
        RX_state = WAIT_START_BYTE;
        break;
    }
    }
}

/*--------------------------------------------------------------------------------
Function: isInvalidDataPackage
Purpose: Check the invalid of the data package
Parameters: DATAPACKAGE*
Return: BOOL_INVALID
--------------------------------------------------------------------------------*/
BOOL_INVALID IsInvalidDataPackage(DATAPACKAGE *mesg)
{
    if ((mesg->data[0] == START_BYTE) && (mesg->data[mesg->length - 1] == STOP_BYTE))
    {
        return DATA_OK;
    }
    return DATA_ERROR;
}
/*--------------------------------------------------------------------------------
Function    : EncodeDataPackage
Purpose     : Encode data from buff to message (msg)
Parameters  : void* buff, void* mesg
Return      : NULL
--------------------------------------------------------------------------------*/
void EncodeDatPackage(void *Buff, DATAPACKAGE *pMesg, REPEAT_BOOL repeat)
{
    uint8_t i = 1; // Index of pMesg, start at 1, pMesg[0] = START_uint8_t
    uint8_t *pBuff = (uint8_t *)Buff;
    pMesg->crc = crc32(pBuff, strlen((char *)pBuff));
    if (repeat == REPEAT)
    {
        pMesg->data[0] = REPEAT_BYTE;
    }
    else
    {
        pMesg->data[0] = START_BYTE;
    }
    while (*pBuff != '\0')
    {
        if (*pBuff == START_BYTE || *pBuff == STOP_BYTE || *pBuff == ESC_BYTE)
        {
            pMesg->data[i++] = ESC_BYTE;
        }
        pMesg->data[i++] = *pBuff;
        pBuff++;
    }
    pMesg->data[i++] = STOP_BYTE;
    pMesg->length = i;
    pMesg->data[i] = '\0';
}

/*--------------------------------------------------------------------------------
Function    : DecodeDataPackage
Purpose     : Decode data from mesg to an array
Parameters  : DATAPACKAGE* mesg, void* resul
Return      : NULL
--------------------------------------------------------------------------------*/
void DecodeDataPackage(DATAPACKAGE *mesg, void *result)
{
    uint8_t *pData = (uint8_t *)result;
    uint8_t nIndex = 1; // Start index is 1

    for (nIndex = 1; nIndex < mesg->length - 1; nIndex++)
    {
        if (mesg->data[nIndex] == ESC_BYTE)
        {
            *pData = mesg->data[++nIndex];
        }
        else
        {
            *pData = mesg->data[nIndex];
        }
        pData++;
    }
    *pData = '\0';
    //}
}