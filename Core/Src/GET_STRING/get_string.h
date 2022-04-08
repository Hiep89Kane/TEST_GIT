#ifndef __GET_STRING_
#define __GET_STRING_

#include "timer.h"
#include "stdint.h"
#include "string.h"

#define MAX_STRING					2//minximum: 2
#define MAX_LEN_STRING				100
#define TIMEOUT_STRING				20//ms
typedef struct{
	char							string[MAX_STRING][MAX_LEN_STRING];// 2 buffer để nhận dữ liệu 
	uint16_t						id_read;//id đọc
	uint16_t						id_write;//id ghi
	uint16_t						count[MAX_STRING];
	struct timer					timeout;
	//notify
	void							(*void_notify)(char *string, uint16_t size);
	uint8_t							flag_notify:1; //==1 đã init hàm (*void_notify) , ==0 nghĩa là chưa init (ko xử lý)
}get_string_str;

void get_string_get_input(get_string_str *sGS, char byte);
void get_string_init_notify(get_string_str *sGS, void (*void_get_string_notify)(char *string, uint16_t size));
void get_string_loop_manage(get_string_str *sGS);

#endif	//__GET_STRING_
