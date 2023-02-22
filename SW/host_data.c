#include "host_data.h"


struct min_context min_ctx;
Clock_State_Frame_t Clock_State_Frame;

void min_send_data(void *data, uint8_t len)
{
    min_send_frame(&min_ctx, MIN_ID, (uint8_t *)data, len);
}

void clock_send_command(ClockCommnadType clock_cmd_type)
{
    ClockCommand cmd;
    cmd.command_id = CLOCK_COMMAND;
    cmd.len = 1;
    cmd.clock_cmd = clock_cmd_type;
    min_send_data(&cmd, sizeof(cmd));
}

void clock_sent_start_signal(void)
{
    Clock_State_Frame = CLOCK_START_STATE;
    clock_send_command(START_COMMAND);
}

void clock_send_finish(ClockCommnadType clock_cmd_type)
{
    ClockCommand cmd;
    cmd.command_id = CLOCK_FINISH;
    cmd.len = 1;
    cmd.clock_cmd = clock_cmd_type;
    min_send_data(&cmd, sizeof(cmd));
}

void clock_send_response(ClockResponse clock_response_type)
{
    ClockResponseFrame response;
    response.command_id = CLOCK_COMMAND;
    response.len = 1;
    response.ack = clock_response_type;
    min_send_data(&response, sizeof(response));
}

void clock_send_data(uint8_t *data, uint8_t len)
{
    ClockData clockData;
    clockData.command_id = CLOCK_DATA;
    clockData.len = len;
    memcpy(&clockData.data, data, len); // ham nay copy den do dai len
    min_send_data(&clockData, sizeof(clockData));
}

// void min_application_handler(uint8_t min_id, uint8_t const *min_payload, uint8_t len_payload, uint8_t port)
// {
//     uint8_t hex_data[21]; // do dai cua 1 phan tu mang hex
//     switch (Clock_State)
//     {
//     case CLOCK_IDLE_STATE:
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx
//         ClockCommand *Clock_command_rx = (ClockCommand *)min_payload;
//         if (Clock_command_rx->command_id == CLOCK_COMMAND && Clock_command_rx->clock_cmd == START_COMMAND)
//         {
//             clock_send_response(ACK);
//             Clock_State = CLOCK_STM32_SEND_STATE;
//         }
//         break;
//         // }
//     case CLOCK_STM32_SEND_STATE: // truyen dong thu 2 tro di
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx
//         ClockResponseFrame *Clock_response_rx3 = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx3->ack == ACK)
//         {
//             char *token = strtok(NULL, "\n");
//             // Serial.println(token);
//             if (token != NULL)
//             {

//                 convert_string_to_array_hex(token, hex_data);
//                 if (hex_data[3] == 0x00) // check record type cua mang
//                 {
//                     if (check_some(hex_data, hex_data[0] + 5) == 1)
//                     {
//                         // Clock_State = Clock_END_STATE;
//                     }
//                     // swap_4_byte(&hex_data[4], hex_data[0]); // swap 4 byte data
//                     clock_send_data(&hex_data[4], hex_data[0]);
//                 }
//                 else
//                 {
//                     clock_send_command(STOP_COMMAND);
//                     Clock_State = CLOCK_IDLE_STATE;
//                 }
//             }
//             else
//             {
//                 clock_send_finish(STOP_COMMAND);
//                 Clock_State = CLOCK_IDLE_STATE;
//             }
//         }
//         else
//         { // nack
//           // Clock_State = Clock_IDLE_STATE;
//         }
//         break;
//     }
//     case CLOCK_START_STATE:
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx

//         ClockResponseFrame *Clock_response_rx = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx->ack == ACK)
//         {
//             if (clock_data.type == "Setting")
//             {
//                 clock_send_command(SETTING_COMMAND);
//                 Clock_State = CLOCK_SETTING_STATE;
//             }
//             else if (clock_data.type == "Alarm")
//             {
//                 clock_send_command(ALARM_COMMAND);
//                 Clock_State = CLOCK_ALARM_STATE;
//             }
//         }
//         else
//         { // nack
//           // Clock_State = Clock_IDLE_STATE;
//         }

//         break;
//     }
//     case CLOCK_SETTING_STATE: // truyen dong thu 1 cua data
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx
//         ClockResponseFrame *Clock_response_rx2 = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx2->ack == ACK)
//         {
//         }
//         else
//         { // nack
//           // Clock_State = Clock_IDLE_STATE;
//         }
//         break;
//     }
//     case CLOCK_ALARM_STATE: // truyen dong thu 1 cua data
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx
//         ClockResponseFrame *Clock_response_rx2 = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx2->ack == ACK)
//         {
//             char *token = strtok(line, "\n"); // dong dau
//             if (token != NULL)
//             {
//                 char *token = strtok(NULL, "\n"); // bo qua dong dau lay dong 2 tro di
//                 // Serial.println(token);
//                 convert_string_to_array_hex(token, hex_data);
//                 check_some(hex_data, hex_data[0] + 5);
//                 // swap_4_byte(&hex_data[4], hex_data[0]); // swap 4 byte data
//                 clock_send_data(&hex_data[4], hex_data[0]);
//                 Clock_State = CLOCK_SEND_DATA_STATE;
//             }
//         }
//         else
//         { // nack
//           // Clock_State = Clock_IDLE_STATE;
//         }
//         break;
//     }
//     case CLOCK_SEND_DATA_STATE: // truyen dong thu 2 tro di
//     {
//         // mapping so byte tu payload vao struct Clock_command_rx
//         ClockResponseFrame *Clock_response_rx3 = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx3->ack == ACK)
//         {
//             char *token = strtok(NULL, "\n");
//             // Serial.println(token);
//             if (token != NULL)
//             {

//                 convert_string_to_array_hex(token, hex_data);
//                 if (hex_data[3] == 0x00) // check record type cua mang
//                 {
//                     if (check_some(hex_data, hex_data[0] + 5) == 1)
//                     {
//                         // Clock_State = Clock_END_STATE;
//                     }
//                     // swap_4_byte(&hex_data[4], hex_data[0]); // swap 4 byte data
//                     clock_send_data(&hex_data[4], hex_data[0]);
//                 }
//                 else
//                 {
//                     clock_send_command(STOP_COMMAND);
//                     Clock_State = CLOCK_IDLE_STATE;
//                 }
//             }
//             else
//             {
//                 clock_send_finish(STOP_COMMAND);
//                 Clock_State = CLOCK_IDLE_STATE;
//             }
//         }
//         else
//         { // nack
//           // Clock_State = Clock_IDLE_STATE;
//         }
//         break;
//     }
//     case CLOCK_END_STATE:
//     {
//         ClockResponseFrame *Clock_response_rx = (ClockResponseFrame *)min_payload;
//         if (Clock_response_rx->ack == ACK)
//         {
//             // Clock_State = Clock_IDLE_STATE;
//         }
//     }
//     default:
//         break;
//     }
//     }
// }

void clock_min_handler(void)
{
	uint8_t data;
	if(uart_available()){	
		data = uart_read();
		min_poll(&min_ctx, &data,1);//dua du lieu vao xem co hop le hay k
		//len = 1;
	}
}

void clock_min_init(void)
{

    min_init_context(&min_ctx, MIN_PORT);
    Clock_State_Frame = CLOCK_IDLE_STATE;
		response_print("STM32 RESET !!");
}
