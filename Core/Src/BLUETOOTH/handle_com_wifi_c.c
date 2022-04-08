#include "handle_com_wifi_c.h"

//khai báo con trỏ hàm void_notify
void (*void_notify)(button_status_t btn_tmp, uint8_t lever) = NULL;

//Init gán con trỏ hàm void_notify
void call_back(void (*notify)(button_status_t btn_tmp, uint8_t lever))
{
	void_notify = notify;
}

int raw_data(com_protocol_t *data, uint32_t length)
{
	static button_status_t bt_save;
	button_status_t bt_detect;
	// nếu lỗi quá độ dài
	if (length > COM_PROTOCOL_LEN)
	{
		dbg_com("length error %u", length);
		return -1;
	}
	// nếu lỗi sai format
	if (data->STX != 0x02 || data->ETX != 0x03)
	{
		dbg_com("header error %u-%u", data->STX, data->ETX);
		return -1;
	}
	// checksum
	if (data->checksum != checksum((uint8_t *)&data->data, sizeof(data->data)))
	{
		dbg_com("crc error %u-%u", data->checksum, checksum((uint8_t *)&data->data, sizeof(data->data)));
		return -1;
	}

	switch (data->cmd)
	{
	case com_wifi_io:
		dbg_com("IO: %u\r\n", data->data.button);

		bt_detect.value = bt_save.value ^ data->data.button.value;
		bt_save.value = data->data.button.value;
		if (bt_detect.value != 0 && void_notify != NULL)
			void_notify(bt_detect, bt_detect.value & bt_save.value); //gọi con trỏ hàm xử lý nút nhấn thay đổi => quá hay

		// if (data->data.button._01)
		// 	dbg_com("button 1 press\r\n");
		// if (data->data.button._02)
		// 	dbg_com("button 2 press\r\n");
		// if (data->data.button._03)
		// 	dbg_com("button 3 press\r\n");
		// if (data->data.button._04)
		// 	dbg_com("button 4 press\r\n");
		// if (data->data.button._05)
		// 	dbg_com("button 5 press\r\n");
		break;
	case com_wifi_temp:
		dbg_com("temprature: %u\r\n", data->data.temprature);
		break;
	default:
		break;
	}
	return 1;
}

uint8_t checksum(uint8_t *data, uint16_t length)
{
	uint8_t crc = 0;
	for (uint16_t i = 0; i < length; i++)
		crc ^= data[i];
	return crc;
}
