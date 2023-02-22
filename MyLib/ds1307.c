#include "ds1307.h"
extern I2C_HandleTypeDef hi2c1; //Tim bien trong cac file .c trong project cua minh

uint8_t BcdToDec(uint8_t num){
	return (num>>4)*10 + (num&0x0F);
}

uint8_t DecToBcd(uint8_t num){
	return (num/10)<<4|(num%10);
}
void RTC_Write_Time(Data_time_t *dt){
	uint8_t Data[8];
	Data[0] = 0x00;
	Data[1] = DecToBcd(dt->sec);
	Data[2] = DecToBcd(dt->min);
	Data[3] = DecToBcd(dt->hour);
	Data[4] = DecToBcd(dt->day);
	Data[5] = DecToBcd(dt->date);
	Data[6] = DecToBcd(dt->month);
	Data[7] = DecToBcd(dt->year);
	HAL_I2C_Master_Transmit(&hi2c1,DS1307_ADDRESS,Data,8,100);
}

void RTC_Write_Time_Alarm(Data_time_t *dt_alarm){
	uint8_t Data[8];
	Data[0] = 0x08;
	Data[1] = DecToBcd(dt_alarm->min);
	Data[2] = DecToBcd(dt_alarm->hour);
	Data[3] = DecToBcd(dt_alarm->day);
	Data[4] = DecToBcd(dt_alarm->date);
	Data[5] = DecToBcd(dt_alarm->month);
	Data[6] = DecToBcd(dt_alarm->year);
	HAL_I2C_Master_Transmit(&hi2c1,DS1307_ADDRESS,Data,7,100);
}

void RTC_Write_State_Relay(State_Relay_t *state_relay){
	uint8_t Data[2];
	Data[0] = 0x0E;
	Data[1] = DecToBcd(state_relay->state);
	HAL_I2C_Master_Transmit(&hi2c1,DS1307_ADDRESS,Data,2,100);
}

void RTC_Write_Alarm_Type(Alarm_type_save_t *alarm_type){
	uint8_t Data[2];
	Data[0] = 0x0F;
	Data[1] = DecToBcd(alarm_type->type);
	HAL_I2C_Master_Transmit(&hi2c1,DS1307_ADDRESS,Data,2,100);
}

void RTC_Read_Time(Data_time_t *dt){
	uint8_t Data[7];
	uint8_t add_reg = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS,&add_reg,1,100);
	HAL_I2C_Master_Receive(&hi2c1,DS1307_ADDRESS,Data,7,100);
	dt->sec = BcdToDec(Data[0]);
	dt->min = BcdToDec(Data[1]);
	dt->hour = BcdToDec(Data[2]);
	dt->day = BcdToDec(Data[3]);
	dt->date = BcdToDec(Data[4]);
	dt->month = BcdToDec(Data[5]);
	dt->year = BcdToDec(Data[6]);
}

void RTC_Read_Time_Alarm(Data_time_t *dt_alarm){
	uint8_t Data[7];
	uint8_t add_reg = 0x08;
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS,&add_reg,1,100);
	HAL_I2C_Master_Receive(&hi2c1,DS1307_ADDRESS,Data,6,100);
	dt_alarm->min = BcdToDec(Data[0]);
	dt_alarm->hour = BcdToDec(Data[1]);
	dt_alarm->day = BcdToDec(Data[2]);
	dt_alarm->date = BcdToDec(Data[3]);
	dt_alarm->month = BcdToDec(Data[4]);
	dt_alarm->year = BcdToDec(Data[5]);
}

void RTC_Read_State_Relay(State_Relay_t *state_relay){
	uint8_t Data[1];
	uint8_t add_reg = 0x0E;
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS,&add_reg,1,100);
	HAL_I2C_Master_Receive(&hi2c1,DS1307_ADDRESS,Data,1,100);
	state_relay->state = BcdToDec(Data[0]);
}

void RTC_Read_Alarm_Type(Alarm_type_save_t *alarm_type){
	uint8_t Data[1];
	uint8_t add_reg = 0x0F;
	HAL_I2C_Master_Transmit(&hi2c1, DS1307_ADDRESS,&add_reg,1,100);
	HAL_I2C_Master_Receive(&hi2c1,DS1307_ADDRESS,Data,1,100);
	alarm_type->type = BcdToDec(Data[0]);
}

uint8_t RTC_Read_Day_Of_Week(Data_time_t *dt){
	int d    = dt->date   ; //Day     1-31
	int m    = dt->month    ; //Month   1-12`
	int y    = 2000 + dt->year ; //Year    2013` 

	uint8_t weekday  = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
	return weekday;
}
