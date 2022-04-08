#ifndef INC_AF4_RS232_H_
#define INC_AF4_RS232_H_

#include "AF_Control.h"
#include "Steamer.h"

	//Range: "20" ..."3F"
	#if (_RS232LL_AddrMe==_RS232LL_AddrAf4 || _RS232LL_AddrYou==_RS232LL_AddrAf4)

		#define _RS232LL_AfCmd_ClickSW	         		0x20
		#define _RS232LL_AfCmd_PrepareConfig        	0x21
		#define _RS232LL_AfCmd_CalibCapSS       		0x22
		#define _RS232LL_AfCmd_SetEPPROM1   			0x23
		#define _RS232LL_AfCmd_SetEPPROM2     			0x24
		#define _RS232LL_AfCmd_CtrlFuncDCS     			0x25
		#define _RS232LL_AfCmd_CtrlFuncLeds     		0x26

		#define _RS232LL_AfCmd_RevMassage     			0x27

		#define _RS232LL_AfCmd_RstFactory				0x2F

		#define _RS232LL_AfCmd_GetByte0           		0x30
		#define _RS232LL_AfCmd_GetByte1     			0x31
		#define _RS232LL_AfCmd_GetByte2     			0x32

		//_RS232LL_AfCmd_CtrlFuncDCS
		#define _FUNC_WHIRL								0x10	//Chi kiem tra 4 bit cao
		#define _FUNC_WHIRL_ON							0x1F
		#define _FUNC_WHIRL_OFF							0x10

		#define _FUNC_STEAM								0x30	//Chi kiem tra 4 bit cao
		#define _FUNC_STEAM_ON							0x3F
		#define _FUNC_STEAM_OFF							0x30

		//_RS232LL_AfCmd_CtrlLeds
		#define _FUNC_RGB								0x10	//Chi kiem tra 4 bit cao
		#define _FUNC_RGB_ON							0x1F	//Chi kiem tra 4 bit cao
		#define _FUNC_RGB_OFF							0x10	//Chi kiem tra 4 bit cao

		#define _FUNC_SPOT								0x20	//Chi kiem tra 4 bit cao
		#define _FUNC_SPOT_ON							0x2F
		#define _FUNC_SPOT_OFF							0x20

		/*=== Define kết quả trả về đối với version rs232 cũ (AutoFill1,2,3 system)*/
		#define _RS232_TX_OK							0xFF
		#define _RS232_TX_FAIL							0x00

		/*=== Define kết quả trả về đối với version rs232 mới (AutoFill4 system) */
		#define _RESULT_FUNC_OK							0x0F
		#define _RESULT_FUNC_FAIL						0x00
		#define _RS232_REPONSE_VAL(function, result)	((function & 0xF0) + result)

		typedef enum {
			_RS232LL_NONE,					/*Trạng thái chưa có gì*/
			_RS232LL_START,					/*Trạng thái đã nhận RXD frame từ DCS*/
			_RS232LL_WAIT_GETSTT,			/*Trạng thái chờ thực thi để lấy dữ liệu reply cho DCS*/
			_RS232LL_WAIT_FINISHED,			/*Trạng thái trạng thái đã thực thi xong => chuẩn bị Reply cho DCS*/
		} RS232LLState_t;

		typedef struct {
			RS232LLState_t 	state;
			uint8_t			f_Skip_AutoRep ; 	//set cờ này lên để bỏ qua tự động update trạng thái lên DCS

			//Rx
			uint8_t 		RxCmd,
							RxData;
			//Tx
			uint8_t 		Backup_RxCmd, 		//luu tru byte cmd =>sau khi thuc hien xong thi gui lai DCS
							TxData;				//gán giá trị mới để gửi đi


		}RS232_Struct_t;

		/*global variables*/
		extern RS232_Struct_t	mainRS232;
		extern const uint8_t 	RxPowerCtrl[_DEF_POWER_TOTAL_NUMMER+1];

		void 	AF4_RS232_Main_exe();//put in main

		void 	AF4_RS232_Task(RS232_Struct_t *RS232_tmp, AF_Struct_t *AF_Tmp, STEAMER_Struct_t *Steamer);
		void	AF4_RS232_UpdateStt(RS232_Struct_t *RS232_tmp, AF_Struct_t *AF_Tmp, STEAMER_Struct_t *Steamer);

	#endif

#endif /* INC_AF4_RS232_H_ */
