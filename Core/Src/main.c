/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#include "Delay.h"
#include "Button.h"
#include "ds1307.h"
#include "string.h"
#include "stdio.h"
#include "ssd1306.h"
#include "fonts.h"
#include "flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
//#include "semphr.h"
//#include "Timers.h"

#include "json_parser.h"
#include "json_generator.h"
//#include "min.h"
//#include "host_data.h"
#include "datapackProc.h"
//#include "alarm_command.h"
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t temp_state_relay = 0;
DATAPACKAGE package;
DATAPACKAGE package_tx;
DATAPACKAGE package_rx;
DATAPACKAGE package_repeat;
char buffer_rx[108];
//char buffer_tx[108];
#define STM32_RX_COMPLETE_BIT 0x00000001
#define STM32_TX_READLY_BIT 0x00000002
#define BUTTON_TRIGGER 0x00000003
typedef enum
{
		NONE,
    UP,
    OK,
    DOWN,
} Button_State_t;
Button_State_t Button_State = NONE; 
static EventGroupHandle_t stm32_button_event_group = NULL;
static EventGroupHandle_t mqtt_event_data_rx = NULL;
static EventGroupHandle_t stm32_event_data_tx = NULL;
typedef enum
{
    IDLE_TX_STATE,
    START_TX_STATE,
    REPEAT_TX_STATE,
    STOP_TX_STATE,
} FarmeData_TX_State_t;

typedef enum
{
    IDLE_RX_STATE,
    START_RX_STATE,
    REPEAT_RX_STATE,
    STOP_RX_STATE,
} FarmeData_RX_State_t;
FarmeData_TX_State_t FarmeData_TX_State = IDLE_TX_STATE;
FarmeData_RX_State_t FarmeData_RX_State = IDLE_RX_STATE;

State_Relay_t state_relay;

typedef struct
{
    char time[10];
    char date[12];
    char state[10];
    char type[10];
    char command[10];
} clock_data_t;

clock_data_t clock_data_rx;
clock_data_t clock_data_tx;
Data_time_t dt;
Data_time_t dt_update;
Data_time_t dt_alarm ;
Button_Typdef btn_down, btn_ok, btn_up;
I2C_HandleTypeDef hi2c1;
static uint32_t temp_tick = 0;

const char* Arr_Day[] = {"SUN", "MON", "TUE", "WED", "THU" , "FRI", "SAT"};//mang 2 chieu

typedef enum {
	NORMAL,
	SETTING,
	ALARM
}Clock_type_t;
Clock_type_t Clock_type = NORMAL;

typedef enum {
	NORMAL_STATE,
	SETTING_HOUR_STATE,
	SETTING_MIN_STATE,
	SETTING_SEC_STATE,
	SETTING_DATE_STATE,
	SETTING_MONTH_STATE,
	SETTING_YEAR_STATE,
	ALARM_HOUR_STATE,
	ALARM_MIN_STATE,
	ALARM_DATE_STATE,
	ALARM_MONTH_STATE,
	ALARM_YEAR_STATE,
}ClockState_t;

typedef enum {
	ALARM_NOTTHING,
	ALARM_SET_OFF,
	ALARM_SET_ON,
}Alarm_type_t;

Alarm_type_t alarmType = ALARM_NOTTHING;

Alarm_type_save_t Alarm_type_save;
ClockState_t ClockState = NORMAL_STATE;
/*=========================utils===============================================*/
uint8_t check_leap_year(uint8_t year){
	if (((year % 4 == 0) && (year % 100!= 0)) || (year%400 == 0)){
		return 1;
	}
	return 0;
}

uint8_t getMaxDay(uint8_t mon, uint8_t year){
	switch(mon){
		case 1:case 3: case 5: case 7: case 8:case 10: case 12:
					return 31;
		case 4: case 6: case 9: case 11:
					return 30;
		case 2:
			if(check_leap_year(year)){
				return 29;
			}
	}
	return 28;
}

/*==========================*/

void OLED_Print_Custom(char *title,Data_time_t *data){
		char str0[16];
		char str[16];
		char str2[16];
		uint8_t day_of_week = RTC_Read_Day_Of_Week(data);
		sprintf(str0," %s       %s",title,Arr_Day[day_of_week]);
		SSD1306_GotoXY (0, 0);
		SSD1306_Puts (str0, &Font_7x10, 1);
		sprintf(str,"%02d:%02d:%02d",data->hour,data->min,data->sec);
		SSD1306_GotoXY (0, 20);
		SSD1306_Puts (str, &Font_11x18, 1);
		sprintf(str2,"%02d/%02d/%04d",data->date,data->month, 2000 + data->year);
		SSD1306_GotoXY (0, 40);
		SSD1306_Puts (str2, &Font_11x18, 1);
		SSD1306_UpdateScreen(); //display
}

void OLED_Print_Custom_Blink(char *title,Data_time_t *data){
		char str0[16];
		char str[16];
		char str2[16];
		static uint8_t is_show = 1;
		uint8_t day_of_week = RTC_Read_Day_Of_Week(data);
		sprintf(str0," %s     %s",title,Arr_Day[day_of_week]);
		SSD1306_GotoXY (0, 0);
		SSD1306_Puts (str0, &Font_7x10, 1);
		sprintf(str,"%02d:%02d:%02d",data->hour,data->min,data->sec);
		SSD1306_GotoXY (0, 20);
		SSD1306_Puts (str, &Font_11x18, 1);
		sprintf(str2,"%02d/%02d/%04d",data->date,data->month, 2000 + data->year);
		SSD1306_GotoXY (0, 40);
		SSD1306_Puts (str2, &Font_11x18, 1);
		SSD1306_UpdateScreen(); //display
		is_show =!is_show;
		if(is_show){
				switch(ClockState){
					case SETTING_HOUR_STATE:
						str[0]=str[1]=' ';
					break;
					case SETTING_MIN_STATE:
						str[3]=str[4]=' ';
					break;
					case SETTING_DATE_STATE:
						str2[0]=str2[1]=' ';
					break;
					case SETTING_MONTH_STATE:
						str2[3]=str2[4]=' ';
					break;
					case SETTING_YEAR_STATE:
						str2[6]=str2[7]=str2[8]=str2[9]=' ';
					break;
					default : break;
				}	
		SSD1306_GotoXY (0, 20);
		SSD1306_Puts (str, &Font_11x18, 1);
		SSD1306_GotoXY (0, 40);
		SSD1306_Puts (str2, &Font_11x18, 1);
		SSD1306_UpdateScreen(); //display
			}
}

void OLED_Print_Custom_Blink_Alarm(char *title,Data_time_t *data){
		char str0[16];
		char str[16];
		char str2[16];
		static uint8_t is_show = 1;
		uint8_t day_of_week = RTC_Read_Day_Of_Week(data);
		sprintf(str0," %s   %s",title,Arr_Day[day_of_week]);
		SSD1306_GotoXY (0, 0);
		SSD1306_Puts (str0, &Font_7x10, 1);
		sprintf(str,"%02d:%02d",data->hour,data->min);
		SSD1306_GotoXY (0, 20);
		SSD1306_Puts (str, &Font_11x18, 1);
		sprintf(str2,"%02d/%02d/%04d",data->date,data->month, 2000 + data->year);
		SSD1306_GotoXY (0, 40);
		SSD1306_Puts (str2, &Font_11x18, 1);
		SSD1306_UpdateScreen(); //display
		is_show =!is_show;
		if(is_show){
				switch(ClockState){
					case ALARM_HOUR_STATE:
						str[0]=str[1]=' ';
					break;
					case ALARM_MIN_STATE:
						str[3]=str[4]=' ';
					break;
					case ALARM_DATE_STATE:
						str2[0]=str2[1]=' ';
					break;
					case ALARM_MONTH_STATE:
						str2[3]=str2[4]=' ';
					break;
					case ALARM_YEAR_STATE:
						str2[6]=str2[7]=str2[8]=str2[9]=' ';
					break;
					default : break;
				}	
		SSD1306_GotoXY (0, 20);
		SSD1306_Puts (str, &Font_11x18, 1);
		SSD1306_GotoXY (0, 40);
		SSD1306_Puts (str2, &Font_11x18, 1);
		SSD1306_UpdateScreen(); //display
			}
}

void Setting_Blink(void){
		static uint32_t t_blink =0;
		char title[] = "SETTING";
		if(HAL_GetTick() - t_blink >= 400){
			RTC_Read_Time(&dt);
			OLED_Print_Custom_Blink(title,&dt);		
			t_blink = HAL_GetTick();
		}
		if(HAL_GetTick() - temp_tick > 5000){
				ClockState = NORMAL_STATE;
		}
}

/*================CALLBACK FUNCTION.======================*/

void time_update(){
		static uint32_t temp_tick = 0;
		char title[] ="CLOCK"; 
		//if(HAL_GetTick() - temp_tick >=300){
			RTC_Read_Time(&dt);
			OLED_Print_Custom(title,&dt);	
			//temp_tick = HAL_GetTick();
		//}

}

 void	Button_Up_Handle(void)
{
		temp_tick = HAL_GetTick();
		switch(ClockState){
			case SETTING_HOUR_STATE:
				dt.hour++;
				if(dt.hour > 23){dt.hour=0; }
				RTC_Write_Time(&dt);
				break;
			case SETTING_MIN_STATE:
				dt.min++;
				if(dt.min > 59){
					dt.min = 0;
				}
				RTC_Write_Time(&dt);
				break;
			case SETTING_DATE_STATE:
				{
				dt.date++;
				uint8_t max_day = getMaxDay(dt.month, dt.year);
				if(dt.date > max_day){
					dt.date = 1;
				}
				RTC_Write_Time(&dt);
				break;
				}
			case SETTING_MONTH_STATE:
				{
				dt.month++;
				if(dt.month > 12){
					dt.month = 1;
				}
				if(dt.date > getMaxDay(dt.month, dt.year)){
					dt.date = getMaxDay(dt.month, dt.year);
				}
				RTC_Write_Time(&dt);
				break;
				}
			case SETTING_YEAR_STATE:
				{
				dt.year++;
				RTC_Write_Time(&dt);
				break;
					}
			case ALARM_HOUR_STATE:
				{
				dt_alarm.hour++;
				if(dt_alarm.hour > 23){dt_alarm.hour=0; }
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_MIN_STATE:
				{
				dt_alarm.min++;
				if(dt_alarm.min > 59){
					dt_alarm.min = 0;
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
				
			case ALARM_DATE_STATE:
				{
				dt_alarm.date++;
				uint8_t max_day = getMaxDay(dt_alarm.month, dt_alarm.year);
				if(dt_alarm.date > max_day){
					dt_alarm.date = 1;
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_MONTH_STATE:
				{
				dt_alarm.month++;
				if(dt_alarm.month > 12){
					dt_alarm.month = 1;
				}
				if(dt_alarm.date > getMaxDay(dt_alarm.month, dt_alarm.year)){
					dt_alarm.date = getMaxDay(dt_alarm.month, dt_alarm.year);
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_YEAR_STATE:
				{
				dt_alarm.year++;
				RTC_Write_Time_Alarm(&dt_alarm);
				break;				
				}
			default: break;
		}
}
void Button_Down_Handle(void) {
		temp_tick = HAL_GetTick();
		switch(ClockState){
			case SETTING_HOUR_STATE:
				{
				dt.hour--;
				if(dt.hour >23 ){dt.hour=23; }
				RTC_Write_Time(&dt);
				break;
			case SETTING_MIN_STATE:
				{
				dt.min--;
				if(dt.min >59){
					dt.min = 59;
				}
			RTC_Write_Time(&dt);
				}
			break;
			case SETTING_DATE_STATE:
				{
				dt.date--;
				uint8_t max_day = getMaxDay(dt.month, dt.year);
				if(dt.date > max_day){
					dt.date = max_day;
				}
				RTC_Write_Time(&dt);
				break;
				}
			case SETTING_MONTH_STATE:
				{
				dt.month--;
				if(dt.month > 12){
					dt.month = 12;
				}
				if(dt.date > getMaxDay(dt.month, dt.year)){
					dt.date = getMaxDay(dt.month, dt.year);
				}
				RTC_Write_Time(&dt);
				}
				break;
			case SETTING_YEAR_STATE:
				dt.year--;
				RTC_Write_Time(&dt);
				break;
			case ALARM_HOUR_STATE:
				{
				dt_alarm.hour--;
				if(dt_alarm.hour > 23){dt_alarm.hour=23; }
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
			case ALARM_MIN_STATE:
				dt_alarm.min--;
				if(dt_alarm.min > 59){
					dt_alarm.min = 59;
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_DATE_STATE:
				{
				dt_alarm.date--;
				uint8_t max_day = getMaxDay(dt_alarm.month, dt_alarm.year);
				if(dt_alarm.date > max_day){
					dt_alarm.date = max_day;
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_MONTH_STATE:
				{
				dt_alarm.month--;
				if(dt_alarm.month > 12){
					dt_alarm.month = 12;
				}
				if(dt_alarm.date > getMaxDay(dt_alarm.month, dt_alarm.year)){
					dt_alarm.date = getMaxDay(dt_alarm.month, dt_alarm.year);
				}
				RTC_Write_Time_Alarm(&dt_alarm);
				break;
				}
			case ALARM_YEAR_STATE:
				dt_alarm.year--;
				RTC_Write_Time_Alarm(&dt_alarm);
				break;	
			default: break;
		}
	}
}
 void btn_press_short_callback(Button_Typdef *ButtonX )
{
	if(ButtonX == &btn_ok){
		Button_State = NONE;
		xEventGroupClearBits(stm32_button_event_group, BUTTON_TRIGGER);
		temp_tick = HAL_GetTick();
		//timeout_alarm_setting(4000);
		switch(ClockState){
			case NORMAL_STATE:
				ClockState = SETTING_HOUR_STATE;
			break;
			case SETTING_HOUR_STATE:
				ClockState = SETTING_MIN_STATE;
				break;
			case SETTING_MIN_STATE:
				ClockState = SETTING_DATE_STATE;
			break;
			case SETTING_DATE_STATE:
				ClockState = SETTING_MONTH_STATE;
				break;
			case SETTING_MONTH_STATE:
				ClockState = SETTING_YEAR_STATE;
				break;
			case SETTING_YEAR_STATE:
				ClockState = NORMAL_STATE;
				Clock_type = SETTING;
				break;
			case ALARM_HOUR_STATE:
					ClockState = ALARM_MIN_STATE;
				break;
			case ALARM_MIN_STATE:
					ClockState = ALARM_DATE_STATE;
				break;
			case ALARM_DATE_STATE:
					ClockState = ALARM_MONTH_STATE;
				break;
			case ALARM_MONTH_STATE:
					ClockState = ALARM_YEAR_STATE;
				break;	
			case ALARM_YEAR_STATE:				
					RTC_Write_Time_Alarm(&dt_alarm);
					xEventGroupSetBits(stm32_event_data_tx, STM32_TX_READLY_BIT);
					FarmeData_TX_State = START_TX_STATE;
					Clock_type = ALARM;
					ClockState = NORMAL_STATE;
				break;				
			default: break;
		}
	}
}

void alarm_screen(){
		static uint32_t t_blink =0;
		char str3[10];
		if(alarmType == ALARM_SET_ON){
			sprintf(str3,"%s", "ALARM ON ");
		}else if(alarmType == ALARM_SET_OFF){sprintf(str3,"%s", "ALARM OFF");}
		if(HAL_GetTick() - t_blink >= 400){
			RTC_Read_Time_Alarm(&dt_alarm);
			OLED_Print_Custom_Blink_Alarm(str3,&dt_alarm);		
			t_blink = HAL_GetTick();
		}
		if(HAL_GetTick() - temp_tick > 5000){
				ClockState = NORMAL_STATE;
		}
		
}

 void btn_press_timeout_callback(Button_Typdef *ButtonX)
{
	if(ButtonX == &btn_ok){
		temp_tick = HAL_GetTick();
		SSD1306_Clear();
		RTC_Read_Time_Alarm(&dt_alarm);
		alarmType= ALARM_SET_ON;
		ClockState = ALARM_HOUR_STATE;
		Alarm_type_save.type = 3;
		RTC_Write_Alarm_Type(&Alarm_type_save);
		xEventGroupClearBits(stm32_button_event_group, BUTTON_TRIGGER);
		Button_State = NONE;
	}
}

void btn_press_timeout_off_callback(Button_Typdef *ButtonX)
{
	if(ButtonX == &btn_ok){
		temp_tick = HAL_GetTick();
		SSD1306_Clear();
		RTC_Read_Time_Alarm(&dt_alarm);
		alarmType= ALARM_SET_OFF;
		ClockState = ALARM_HOUR_STATE;
		Alarm_type_save.type = 2;
		RTC_Write_Alarm_Type(&Alarm_type_save);
		xEventGroupClearBits(stm32_button_event_group, BUTTON_TRIGGER);
		Button_State = NONE;
	}
}


void check_alarm(){
	RTC_Read_Alarm_Type(&Alarm_type_save);
		RTC_Read_Time_Alarm(&dt_alarm);		
	if(dt_alarm.hour == dt.hour && dt_alarm.min == dt.min 
		&& dt_alarm.month == dt.month && dt_alarm.date == dt.date && dt_alarm.year == dt.year){
			if(Alarm_type_save.type == 3) {
				 HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_RESET);
				Alarm_type_save.type = 0;
				RTC_Write_Alarm_Type(&Alarm_type_save);
				dt_alarm.hour = 0;
				dt_alarm.min=0;
				//dt_alarm.sec = 1;
				dt_alarm.date= 4;
				dt_alarm.month = 11;
				dt_alarm.year = 22;
				RTC_Write_Time_Alarm(&dt_alarm);		
				temp_state_relay = 1;
				//RTC_Write_State_Relay(&state_relay);
				Flash_Erase((uint32_t)0x0801FC00);//page 127
		    Flash_Write_Syn((uint32_t)0x0801FC00,&temp_state_relay,2);
			}else if(Alarm_type_save.type == 2){
					HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_SET);
					Alarm_type_save.type = 0;
					RTC_Write_Alarm_Type(&Alarm_type_save);
					dt_alarm.hour = 0;
					dt_alarm.min=0;
					//dt_alarm.sec = 1;
					dt_alarm.date= 11;
					dt_alarm.month = 4;
					dt_alarm.year = 22;
					RTC_Write_Time_Alarm(&dt_alarm);
					temp_state_relay = 0;
					//RTC_Write_State_Relay(&state_relay);	
					Flash_Erase((uint32_t)0x0801FC00);//page 127
		    Flash_Write_Syn((uint32_t)0x0801FC00,&temp_state_relay,2);
			}
	}
}
/*====================HANDLE CLOCK=========================*/
void vTaskClockHandle(void *pvParameters){
	TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 500;
    xLastWakeTime = xTaskGetTickCount();
	while(1){
			switch(ClockState){
		case NORMAL_STATE:
			time_update();
			check_alarm();
			break;
		case SETTING_HOUR_STATE:
		case SETTING_MIN_STATE:
		case SETTING_DATE_STATE:
		case SETTING_MONTH_STATE:
		case SETTING_YEAR_STATE:
			Setting_Blink();
			break;
		case ALARM_HOUR_STATE:
		case ALARM_MIN_STATE:
		case ALARM_MONTH_STATE:
		case ALARM_DATE_STATE:
		case ALARM_YEAR_STATE:
			{
			alarm_screen();
				break;
			}
		default: break;
		}
			vTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}
//void clock_handle(void){
//	switch(ClockState){
//		case NORMAL_STATE:
//			time_update();
//			check_alarm();
//			break;
//		case SETTING_HOUR_STATE:
//		case SETTING_MIN_STATE:
//		case SETTING_DATE_STATE:
//		case SETTING_MONTH_STATE:
//		case SETTING_YEAR_STATE:
//			Setting_Blink();
//			break;
//		case ALARM_HOUR_STATE:
//		case ALARM_MIN_STATE:
//		case ALARM_MONTH_STATE:
//		case ALARM_DATE_STATE:
//		case ALARM_YEAR_STATE:
//			{
//			alarm_screen();
//				break;
//			}
//		default: break;
//	}
//}
/*==================HANDLE BUTTON==================*/
uint8_t flag_Interrup = 0;
uint32_t time_delay = 0;
void EXTI0_IRQHandler(void)
{
	if (EXTI->PR & (1<<0))    // If the PA0 triggered the interrupt
	{
		if(HAL_GetTick() - time_delay >=30)
		{
				if(temp_state_relay == 1){
				temp_state_relay = 0;
				HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_SET);
		}else if(temp_state_relay == 0){
				HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_RESET);
				temp_state_relay = 1;
		}
		Flash_Erase((uint32_t)0x0801FC00);//page 127
		Flash_Write_Syn((uint32_t)0x0801FC00,(uint8_t*)&temp_state_relay,2);
		//RTC_Write_State_Relay(&state_relay);
		time_delay = HAL_GetTick();
		}

		EXTI->PR |= (1<<0);  // Clear the interrupt flag by writing a 1 
	}
}

void EXTI4_IRQHandler(void)
{
	if (EXTI->PR & (1<<4))    // If the PA5 triggered the interrupt
	{
		//xEventGroupSetBits(stm32_button_event_group, BUTTON_TRIGGER);
		Button_State = DOWN;
		EXTI->PR |= (1<<4);  // Clear the interrupt flag by writing a 1 
	}
}

void EXTI9_5_IRQHandler(void)
{
	if (EXTI->PR & (1<<5))    // If the PA5 triggered the interrupt
	{
		if(HAL_GetTick() - time_delay >=10)
		{
		//xEventGroupSetBits(stm32_button_event_group, BUTTON_TRIGGER);
		Button_State = OK;
		time_delay = HAL_GetTick();
		}
		EXTI->PR |= (1<<5);  // Clear the interrupt flag by writing a 1 
	}else if (EXTI->PR & (1<<6))    // If the PA6 triggered the interrupt
	{
		//if(HAL_GetTick() - time_delay >=30)
		//{
		//xEventGroupSetBits(stm32_button_event_group, BUTTON_TRIGGER);
		Button_State = UP;
		//time_delay = HAL_GetTick();
		//}
		EXTI->PR |= (1<<6);  // Clear the interrupt flag by writing a 1 
	}
}
void vTaskButtonHandle(void *pvParameters){
		TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 50;
    xLastWakeTime = xTaskGetTickCount();
	while(1){
		//xEventGroupWaitBits(stm32_button_event_group, BUTTON_TRIGGER, pdFALSE, pdFALSE, portMAX_DELAY);
		button_handle(&btn_ok);
		//button_handle(&btn_up);
		//button_handle(&btn_down);
		if(Button_State == DOWN){
			Button_Down_Handle();
			Button_State = NONE;
		}else if(Button_State == UP){
			Button_Up_Handle();
			Button_State = NONE;
		}
		//xEventGroupClearBits(stm32_button_event_group, BUTTON_TRIGGER);
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		//vTaskDelay(50/portTICK_RATE_MS);
	}
}

/*==================UPDATE OVER ESP32==================*/

 uint8_t data_rx[109]; 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == huart2.Instance){
		//uart_receive_rx(data_rx);
		
		DATAPACKAGE *datapackage_RX = (DATAPACKAGE *)&package;
    package_rx = *datapackage_RX;
		//FarmeData_RX_State = START_RX_STATE;
		FarmeData_RX_State = START_RX_STATE;
		xEventGroupSetBits(mqtt_event_data_rx, STM32_RX_COMPLETE_BIT);
		//response_print("%s", (char)data_rx);
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&package, sizeof(package));
	}

}

typedef struct
{
    char buf[100];
    size_t offset;
} json_gen_test_result_t;

static json_gen_test_result_t result;
static void flush_str(char *buf, void *priv)
{
    json_gen_test_result_t *result = (json_gen_test_result_t *)priv;
    if (result)
    {
        if (strlen(buf) > sizeof(result->buf) - result->offset)
        {
            return;
        }
        memcpy(result->buf + result->offset, buf, strlen(buf));
        result->offset += strlen(buf);
    }
}

char *json_gen(json_gen_test_result_t *result, char *key1, char* value1,
               char *key2, char* value2, char *key3, char* value3, char *key4, char* value4, char *key5, char* value5)
{
    char buf[20];
    memset(result, 0, sizeof(json_gen_test_result_t));
    json_gen_str_t jstr;
    json_gen_str_start(&jstr, buf, sizeof(buf), flush_str, result);
    json_gen_start_object(&jstr);
    json_gen_obj_set_string(&jstr, key1, value1);
    json_gen_obj_set_string(&jstr, key2, value2);
		json_gen_obj_set_string(&jstr, key3, value3);
		json_gen_obj_set_string(&jstr, key4, value4);
		json_gen_obj_set_string(&jstr, key5, value5);
    json_gen_end_object(&jstr);
    json_gen_str_end(&jstr);
    return result->buf;
}

int json_parse_clock(char *json, clock_data_t *out_data)
{
    jparse_ctx_t jctx;
    int ret = json_parse_start(&jctx, json, strlen(json));
    if (ret != OS_SUCCESS)
    {
        return -1;
    }
    if (json_obj_get_string(&jctx, "time", out_data->time, 20) != OS_SUCCESS)
    {
    }
    if (json_obj_get_string(&jctx, "date", out_data->date, 20) != OS_SUCCESS)
    {
    }

    if (json_obj_get_string(&jctx, "state", out_data->state, 20) != OS_SUCCESS)
    {
    }
    if (json_obj_get_string(&jctx, "type", out_data->type, 20) != OS_SUCCESS)
    {
    }
    if (json_obj_get_string(&jctx, "command", out_data->command, 20) != OS_SUCCESS)
    {
    }
    json_parse_end(&jctx);
    return 0;
}

void setting_time_network(clock_data_t *clock_data_rx){
	dt_update.sec =0;
	char *temp = strtok(clock_data_rx->time,":");
  dt_update.hour = atoi(temp);
  temp= strtok(NULL,":");
  dt_update.min = atoi(temp);
  temp = strtok(clock_data_rx->date,"-");
  dt_update.year=atoi(temp) -2000;
  temp = strtok(NULL,"-");
  dt_update.month=atoi(temp);
  temp = strtok(NULL,"-");
  dt_update.date=atoi(temp);
	RTC_Write_Time(&dt_update);
}

void setting_alarm_network(clock_data_t *clock_data_rx){
	dt_alarm.sec =0;
	char *temp = strtok(clock_data_rx->time,":");
  dt_alarm.hour = atoi(temp);
  temp= strtok(NULL,":");
  dt_alarm.min = atoi(temp);
  temp = strtok(clock_data_rx->date,"-");
  dt_alarm.year=atoi(temp) -2000;
  temp = strtok(NULL,"-");
  dt_alarm.month=atoi(temp);
  temp = strtok(NULL,"-");
  dt_alarm.date=atoi(temp);
	RTC_Write_Time_Alarm(&dt_alarm);
}

void clear_alarm_network(void){
	dt_alarm.sec =0;
  dt_alarm.hour = 0;
  dt_alarm.min = 0;
  dt_alarm.year=22;
  dt_alarm.date=11;
  dt_alarm.month=3;
	RTC_Write_Time_Alarm(&dt_alarm);
	Alarm_type_save.type = 0;
	RTC_Write_Alarm_Type(&Alarm_type_save);
}

void remote_on_off(char command_remote){
	if(command_remote == '1'){
				HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_RESET);
				temp_state_relay = 1;
	}else if(command_remote == '0'){
				HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_SET);
				temp_state_relay = 0;
	}
	Flash_Erase((uint32_t)0x0801FC00);//page 127
	Flash_Write_Syn((uint32_t)0x0801FC00,(uint8_t*)&temp_state_relay,2);
}
void update_time(char* data){
	json_parse_clock(data, &clock_data_rx);
	if(strstr(clock_data_rx.type,"Setting") !=NULL){
		setting_time_network(&clock_data_rx);
	}else if(strstr(clock_data_rx.type,"Alarm") != NULL && strstr(clock_data_rx.command,"Write") != NULL){
		setting_alarm_network(&clock_data_rx);
		if(strstr(clock_data_rx.state,"ON") != NULL){
			Alarm_type_save.type = 3;
			RTC_Write_Alarm_Type(&Alarm_type_save);
		}else if(strstr(clock_data_rx.state,"OFF") != NULL){
			Alarm_type_save.type = 2;
			RTC_Write_Alarm_Type(&Alarm_type_save);
		}
	}else{
		clear_alarm_network();
	}
}

char alarm_tx[100];
void alarm_package_tx(void){
	char alarm_time[5];
	char alarm_date[10];
	char alarm_state[3];
	char alarm_type[7];
	char alarm_command[5];
	sprintf(alarm_time,"%d:%d",dt_alarm.hour,dt_alarm.min);
	sprintf(alarm_date,"%d-%d-%d",dt_alarm.year+2000,dt_alarm.month,dt_alarm.date);
	if(alarmType == ALARM_SET_OFF){
		strcpy(alarm_state,"OFF");
	}else if(alarmType == ALARM_SET_ON) {
		strcpy(alarm_state,"ON");
	}
	strcpy(alarm_type,"Alarm");
	strcpy(alarm_command,"Write");
	json_gen(&result,"time",alarm_time,"date",alarm_date,"state",alarm_state,"type",alarm_type,"command",alarm_command);
}
void vTaskTxSTM32(void *pvParameters){
    while (1)
    {
				xEventGroupWaitBits(stm32_event_data_tx, STM32_TX_READLY_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        switch (FarmeData_TX_State)
        {
        case IDLE_TX_STATE:
        {

            break;
        }
        case START_TX_STATE:
        {
						alarm_package_tx();
            EncodeDatPackage(result.buf, &package_tx, NOT_REPEAT);
						HAL_UART_Transmit(&huart2,(uint8_t*)&package_tx,sizeof(package_tx),500);
            FarmeData_TX_State = STOP_TX_STATE;
        }
				case REPEAT_TX_STATE:
				{
					break;
				}
        case STOP_TX_STATE:
        {
					xEventGroupClearBits(mqtt_event_data_rx, STM32_TX_READLY_BIT);
            FarmeData_TX_State = IDLE_TX_STATE;
            break;
        }
        default:
            break;
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    //vTaskDelete(NULL);
}

void vTaskRxSTM32(void *pvParameters){
		const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
		char command_remote;
		while(1){
			//Su dung các task huong su kien
			//task can enter the Blocked state to wait for one or more flags within the group to become active
			//N?u xClearOnExit du?c d?t thành pdFALSE thì các bit du?c d?t trong nhóm s? ki?n s? không b? thay d?i khi lenh goi xEventGroupWaitBits() tra ve
		xEventGroupWaitBits(mqtt_event_data_rx, STM32_RX_COMPLETE_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
		switch (FarmeData_RX_State)
        {
        case IDLE_RX_STATE:
        {
            break;
        }
        case START_RX_STATE:
        {
            DecodeDataPackage(&package_rx, buffer_rx);
						if(buffer_rx[0] == REPEAT_BYTE){
							FarmeData_RX_State = REPEAT_RX_STATE;
								break;
						}
            if (package_rx.crc == crc32((uint8_t *)buffer_rx, strlen((char *)buffer_rx)))
            {

                FarmeData_RX_State = STOP_RX_STATE;
            }
            else
            {
								EncodeDatPackage("Repeat", &package_repeat, REPEAT);
								HAL_UART_Transmit(&huart2,(uint8_t*)&package_repeat,sizeof(package_repeat), 500);
                FarmeData_RX_State = IDLE_RX_STATE;
								xEventGroupClearBits(mqtt_event_data_rx, STM32_RX_COMPLETE_BIT);
            }
            break;
        }
        case REPEAT_RX_STATE:
        {
            //EncodeDatPackage(result.buf, &package_tx, NOT_REPEAT);
						//HAL_UART_Transmit(&huart2,(uint8_t*)&package_tx,sizeof(package_tx),500);
            FarmeData_RX_State = IDLE_RX_STATE;
            break;
        }
        case STOP_RX_STATE:
        {
						xEventGroupClearBits(mqtt_event_data_rx, STM32_RX_COMPLETE_BIT);
            FarmeData_RX_State = IDLE_RX_STATE;
						if(strstr(buffer_rx,"ZZZ") != NULL){
							command_remote = buffer_rx[5];
							remote_on_off(command_remote);
						}else{
							update_time(buffer_rx);
						}			
						
            break;
        }
        default:
            break;
        }
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    //vTaskDelete(NULL);
	}	

void PVD_IRQHandler(void)
{
  /* Check if PVD Interrupt has occurred */
		//Flash_BackUp((uint32_t)0x0801FC00, &temp_state_relay);
  	//Flash_Erase((uint32_t)0x0801FC00);//page 127
		//temp_state_relay =3;
		//Flash_Write_Syn((uint32_t)0x0801FC00,&temp_state_relay,2);
}

void PVD_Config(void)
{
  /* Enable PWR clock */
  __HAL_RCC_PWR_CLK_ENABLE();
	
	PWR_PVDTypeDef sConfigPVD;
	 sConfigPVD.PVDLevel = PWR_PVDLEVEL_7;
  sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING_FALLING;
  /* Configure the PVD Level to 2.9V */
  HAL_PWR_ConfigPVD(&sConfigPVD);

  /* Enable the PVD Output */
  HAL_PWR_EnablePVD();

  /* Enable and set PVD Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PVD_IRQn);
}
	
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
	Interrupt_Config();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
	PVD_Config();
  //MX_CRC_Init();
  /* USER CODE BEGIN 2 */
	TIM2_Config();
	SSD1306_Init(); 
	HAL_UART_Receive_IT(&huart2, (uint8_t*)&package, sizeof(package));
//	dt_alarm.hour = 19;
//	dt_alarm.min=44;
//	//dt_alarm.sec = 1;
//	dt_alarm.date= 4;
//	dt_alarm.month = 11;
//	dt_alarm.year = 22;
//	RTC_Write_Time_Alarm(&dt_alarm);

	//dt.hour = 23;
	//dt.min=11;
	//dt.sec = 0;
	//dt.date= 7;
	//dt.month = 2;
	//dt.year = 23;
	//RTC_Write_Time(&dt);
	//temp_state_relay = 1;
	//Flash_Erase((uint32_t)0x0801FC00);//page 127
	//Flash_Write_Syn((uint32_t)0x0801FC00,(uint8_t*)&temp_state_relay,2);
	Flash_Read((uint32_t)0x0801FC00,(uint8_t*)&temp_state_relay,1);
	//RTC_Read_State_Relay(&state_relay);
		if(temp_state_relay == 1){
			  HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_RESET);
		}else if(temp_state_relay == 0){
				HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_SET);
		}
	button_init(&btn_down,GPIOA, GPIO_PIN_4);
	button_init(&btn_ok,GPIOA, GPIO_PIN_5);
	button_init(&btn_up,GPIOA, GPIO_PIN_6);
		mqtt_event_data_rx = xEventGroupCreate();
		stm32_event_data_tx =xEventGroupCreate();
		stm32_button_event_group = xEventGroupCreate();
		xTaskCreate(vTaskTxSTM32,(const char*)"vTaskTxSTM32", configMINIMAL_STACK_SIZE*3, NULL, 3,( xTaskHandle * ) NULL);//Dynamically Allocated RAM
		xTaskCreate(vTaskRxSTM32,(const char*)"smt32_rx_loop", configMINIMAL_STACK_SIZE*2, NULL, 3,( xTaskHandle * ) NULL);
		xTaskCreate(vTaskClockHandle,(const char*)"vTaskClockHandle", configMINIMAL_STACK_SIZE*3, NULL, 6,( xTaskHandle * ) NULL);//muc do uu tien cao nhat
		xTaskCreate(vTaskButtonHandle,(const char*)"vTaskButtonHandle", configMINIMAL_STACK_SIZE*3, NULL,5,( xTaskHandle * ) NULL);
		vTaskStartScheduler();
		//Flash_Erase((uint32_t)0x0801FC00);//page 127
//		if(FLASH_NO_ERRORS != Flash_Write_Syn((uint32_t)0x08001000,aBackUpData_W,LENGBUFFER)){
//			while(1);
//		}
//		
//		if(FLASH_NO_ERRORS != Flash_Read((uint32_t)0x08001000,aBackUpData_R,LENGBUFFER)){
//			while(1);
//		}
//		Flash_Erase((uint32_t)0x08001000);
		//configUSE_PREEMPTION 
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
	return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(relay_output_GPIO_Port, relay_output_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : btn_down_Pin btn_ok_Pin btn_up_Pin */
  GPIO_InitStruct.Pin = btn_down_Pin|btn_ok_Pin|btn_up_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIOA->CRL &= ~(0x0f<<0);  // clear bits (7:6:5:4)
	
	GPIOA->CRL |= (8<<0);  // Bits (7:6:5:4) = 1:0:0:0  --> PA1 in Input Mode in Pull-up/ Pull-down mode
	
	GPIOA->ODR |= (1<<0);  //  --> PA1 is in Pull UP mode
	
  /*Configure GPIO pin : relay_output_Pin */
  GPIO_InitStruct.Pin = relay_output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(relay_output_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
