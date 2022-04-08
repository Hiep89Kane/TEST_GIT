#ifndef INC_BLE_HC05_H_
#define INC_BLE_HC05_H_

#include "myHeader.h"
#include "handle_com_wifi_regs.h"

#define BLE_putc					uart3_putc
#define BLE_puts  					uart3_puts

#define BLE_BUFFER_MAX				20

#define  _STEAM_ON					"STEAMON"
#define  _STEAM_OFF					"STEAMOFF"
#define  _STEAM_ENSAFE				"STEAMENSAFE"
#define  _STEAM_DISSAFE				"STEAMDISSAFE"
#define  _STEAM_SETTEMP				"STEAMSETTEMP"
#define  _STEAM_SETPWR				"STEAMSETPWR"
#define  _STEAM_DEBUGADC			"STEAMDEBUG"

#define  _AUTOFILL_REV				"AUTOFILLREV"
#define  _AUTOFILL_ON				"AUTOFILLON"
#define  _AUTOFILL_OFF				"AUTOFILLOFF"

#define  _MASSAGE_REV				"MASSAGEREV"

typedef enum {
	_BLECTRL_NULL,
	_BLECTRL_STEAM_ON,
	_BLECTRL_KEEPWARM_ON,
	_BLECTRL_STEAM_OFF,
	_BLECTRL_ST_SAFETY_EN,
	_BLECTRL_ST_SAFETY_DIS,
	_BLECTRL_SETTEMPC,
	_BLECTRL_SETPWR,
	_BLECTRL_ADCDEBUG,

	_BLECTRL_AUTOFILL_REV,
	_BLECTRL_AUTOFILL_ON,
	_BLECTRL_AUTOFILL_OFF,
	_BLECTRL_MASSAGE_REV

} BLECtrl_Typedef;

typedef enum {
	_BLE_OK,
	_BLE_ERR = !_BLE_OK
} BLEStt_TypeDef;

typedef enum {
	_BLE_NONE,
	_BLE_BUSY,
	_BLE_FINISHED
} BLERxStt_TypeDef;

/* Global variables ---------------------------------------------------------*/
extern uint8_t BLE_RxBuff[BLE_BUFFER_MAX];
extern BLERxStt_TypeDef BLE_RX_Stt;

void BLE_init();

void BLE_GetBuffer(char RxData);                                                //get in RX irq
BLECtrl_Typedef BLE_GetCtrl(STEAMER_Struct_t *Steamer);
void BLE_Ctrl_Task(uint8_t cmd, STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp);

void WIFI_Ctrl_Task(button_status_t *btn_tmp, uint8_t lever, RS232_Struct_t *RS232_tmp);
void WIFI_Ctrl_Task_Bt(button_status_t btn_tmp, uint8_t lever);

#endif /* INC_BLE_HC05_H_ */
