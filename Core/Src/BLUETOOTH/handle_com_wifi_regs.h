#ifndef _HANDLE_COM_WIFI_REGS_H__
#define _HANDLE_COM_WIFI_REGS_H__

#include "stdint.h"

// loại dữ liệu trao đổi. là button hay nhiệt độ
typedef enum
{
	com_wifi_io = 1,
	com_wifi_temp,
} com_protocol_cmd_e;

// // các loại button
// typedef enum
// {
// 	button_01 = 0x01,
// 	button_02 = 0x02,
// 	button_03 = 0x04,
// 	button_04 = 0x08,
// 	button_05 = 0x10,
// } button_type_t;

//
typedef union
{
	struct
	{
		uint8_t _01 : 1; // lsb
		uint8_t _02 : 1;
		uint8_t _03 : 1;
		uint8_t _04 : 1;
		uint8_t _05 : 1; // msb
	};
	uint8_t value;
} button_status_t;

typedef struct
{
	// ký tự bắt đầu
	uint8_t STX;
	// kiểu dữ liệu
	uint8_t cmd;
	union
	{
		// init union fix length = 4 byte`
		uint8_t unionInit[4];
		// data
		button_status_t button;
		// data nhiệt độ
		uint8_t temprature;
	} data;
	// check toàn vẹn data
	uint8_t checksum;
	// ký tự kết thúc
	uint8_t ETX;
} com_protocol_t;

#define COM_PROTOCOL_LEN sizeof(com_protocol_t)

#endif
