#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include "main.h"

#define CONFIG_ADDR 0x801FC00 //page 128

typedef enum {
	NORMAL,
	UPDATE_OTA
}ResetCause;

typedef enum {
	APP1_ACTIVE,
	APP2_ACTIVE
}RegionActive;

typedef struct {
	RegionActive region_act;
	ResetCause rst_cause;
}__attribute__((packed)) SystemInfo;


#endif
