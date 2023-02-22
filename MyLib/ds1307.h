#ifndef __DS_1307_H_
#define __DS_1307_H_

#include "main.h"
#define DS1307_ADDRESS 0x68<<1
typedef struct{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
}Data_time_t;

typedef struct{
	uint8_t state;
}State_Relay_t;

typedef struct{
	uint8_t type;
}Alarm_type_save_t;

void RTC_Write_Alarm_Type(Alarm_type_save_t *alarm_type);
void RTC_Read_Alarm_Type(Alarm_type_save_t *alarm_type);
void RTC_Read_State_Relay(State_Relay_t *state_relay);
void RTC_Write_State_Relay(State_Relay_t *state_relay);
void RTC_Write_Time(Data_time_t *dt);
void RTC_Read_Time(Data_time_t *dt);
void RTC_Write_Time_Alarm(Data_time_t *dt_alarm);
void RTC_Read_Time_Alarm(Data_time_t *dt_alarm);
uint8_t RTC_Read_Day_Of_Week(Data_time_t *dt);
uint8_t BcdToDec(uint8_t num);
uint8_t DecToBcd(uint8_t num);
#endif
