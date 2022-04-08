#ifndef _HANDLE_COM_WIFI_CH__
#define _HANDLE_COM_WIFI_CH__

#include "handle_com_wifi_regs.h"
#include "printf.h"

//#define dbg_com(fmt, ...) debug_msg(PSTR("\r\n>COM< " fmt), ##__VA_ARGS__)
#define dbg_com(...) debug_msg(__VA_ARGS__)

void call_back(void (*notify)(button_status_t btn_tmp, uint8_t lever));
int raw_data(com_protocol_t *data, uint32_t length);
uint8_t checksum(uint8_t *data, uint16_t length);

#endif
