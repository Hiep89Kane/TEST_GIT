/*
 * AF_Control.h
 *
 *  Created on: Mar 19, 2020
 *      Author: NGUYEN VAN HIEP
 */
/*                                             	|<- 12x4 space>*/
#ifndef INC_AF_CONTROL_H_
#define INC_AF_CONTROL_H_

#include "CtrlRGB.h"

#define AF_WHIRL_On()					wbi(Global_RLAC_Manager,_RLWHIRL_Bit,1)
#define AF_WHIRL_Off()					wbi(Global_RLAC_Manager,_RLWHIRL_Bit,0)

#define AF_DRAIN_On()					wbi(Global_RLAC_Manager,_RLDRAIN_Bit,1)
#define AF_DRAIN_Off()					wbi(Global_RLAC_Manager,_RLDRAIN_Bit,0)

#define AF_Solenoid_On()				do{\
											if(RL_SOL_COM_STT==0){RL_SOL_COM_ON;delay_ms(100);}\
											OUTPUT_set_mode(&_CTRL_SOLAF, _OUTPUT_mode_on);\
										}while(0)

#define AF_Solenoid_Off()				do{\
											OUTPUT_set_mode(&_CTRL_SOLAF, _OUTPUT_mode_off);\
											if(_CTRL_SOLST.mode <= _OUTPUT_mode_off){delay_ms(100); RL_SOL_COM_OFF;}\
										}while(0)

#define _AF_TIME_AUTO_OFF      			(CLOCK_HOUR)						//thời gian AUTO OFF hệ thống
#define _AF_TIME_FILL_MORE				(10*CLOCK_SECOND)					//thời gian xả thêm nước nếu sensor đủ nước
#define _AF_TIME_STOP_FILL(num_S)		(num_S*CLOCK_SECOND)				//thoi gian water level

#define _AF_TIME_PROTECT_OVERFLOW		(5*CLOCK_MINUTE)					//Thời gian tối đa solenoid hoạt động nếu ko thấy nước

#define _AF_TIME_HOLDDRAIN_NOSENSOR		(5*CLOCK_MINUTE)					//Time out tat Drain neu ko nha nut Drain
#define _AF_TIME_HOLDDRAIN_SENSOR		(5*CLOCK_SECOND)					//Timeout chi con 5s khi co Sensor Drain
#define _AF_TIME_HOLDDRAIN_OFFALL		(CLOCK_SECOND)						//Timeout tắt RGB , EcoJet , Steamer sau khi bat drainpump
#define	_TIMEOUT_DEACTIVE_DRAINSS		(30*CLOCK_SECOND)					//sau khi fill nuoc 30s =>Sensor van do =>Sensor kha nang roi ra ngoai
#define	_TIMEOUT_RST_USERS_ACTIVE		(20*CLOCK_SECOND)                   //nhấn giữ Drain > 20s thì reset biến : Users_active = _DEACTIE

#define _AF_TIME_CHECK_CAPSS			(3*CLOCK_SECOND)					//kt trang thai sensor

#define _AF_TIME_JETSW_ON_ECOJET		(5*CLOCK_SECOND)					//giu jet sw de bat whirlpool
#define _AF_TIME_JETSW_HOLD_LIMIT		(12*CLOCK_SECOND)					//time out off để bảo vệ

#define _AF_TIME_FACTORY_RST			(5*CLOCK_SECOND)					//nut nhan reset trên control box

#define _AF_TIME_RESET_RS232 			(12*CLOCK_SECOND)

#define _AF_TIME_RETRY_UPDATE_RS232     (CLOCK_SECOND/2)					//retry luc update trang thai thay doi JET hoac DRAIN neu ko co phan hoi tu DCS

#define _AF_LV_MIN						1
#define _AF_LV_MAX						5
#define _AF_LV_DEFAULT					3
#define _IS_WATERLEVER(val)				(_LIMIT(val, _AF_LV_MIN, _AF_LV_MAX))

#define check_Flash_WaterLV(val)		((_LIMIT(val, _AF_LV_MIN, _AF_LV_MAX)) ? val : _AF_LV_DEFAULT)

typedef enum {
	_AFRGB_OFF,
	_AFRGB_ON = !_AFRGB_OFF,
}AFCtrlRGB_TypeDef_t;

typedef enum {
	_AFCTRLRGB_FULLCOLOR=0,				// 0
	_AFCTRLRGB_WHITE=1,                 // 1
	_AFCTRLRGB_GREEN=2,                 // 2
	_AFCTRLRGB_YELLOW=3,                // 3
	_AFCTRLRGB_RED=4,                   // 4
	_AFCTRLRGB_PINK=5,                 	// 5
	_AFCTRLRGB_BLUE=6,                  // 6
	_AFCTRLRGB_BLUESKY=7,              	// 7

	_AFCTRLRGB_MAX=8,               	// 8
} AFSetRGB_Typedef;

#define _IS_AFCTRLRGB(val)				(_LIMIT(val, _AFCTRLRGB_FULLCOLOR, _AFCTRLRGB_MAX-1))
#define check_Flash_CtrlRGB(val)		((_LIMIT(val, _AFCTRLRGB_FULLCOLOR, _AFCTRLRGB_MAX-1)) ? val : _AFCTRLRGB_FULLCOLOR)

typedef enum {
	_Input_SSDrain_Logic,
	_Input_SSWater_Logic,
	_Input_Safety_Logic,

	_Input_Total
} Input_IndexDef;

enum{
	_BYTE0,
	_BYTE1,
	_BYTE2,
	//..
	_BYTE_MAX
};

//PASS or _FAIL
enum {
	//BYTE 0
	_INFO_SSWATER_CONNECT_Bit,          		//0
	_INFO_SSDRAIN_CONNECT_Bit,                  //1
	_INFO_SSWATER_Logic_Bit,                    //2
	_INFO_SSDRAIN_Logic_Bit,                    //3
	_INFO_AF4_WHIRLON_Bit,						//4 => Set=1 khi đủ nước bật Jet hoặc hold on JetSW > 5s
	_INFO_AF4_LIGHTON_Bit,						//5	=> Set 1 Khi spotligh duoc bat ,0 la tat
	_INFO_RGB_STT_Bit,                          //6 => Set 1 khi bat LED RGB
	_INFO_STEAMER_CTRL_Bit						//7
}AF_infor_byte0;

enum {
	//BYTE 1
	_INFO_RLCOM_Bit,                        	//0
	_INFO_RLWHIRL_Bit,                          //1
	_INFO_RLDRAIN_Bit,                          //2
	_INFO_RLAIR_Bit,                            //3
	_INFO_DIM_POWER_Bit,                        //4
	_INFO_FULL_POWER_Bit,                       //5
	_INFO_SOLAF_Bit,                            //6
	_INFO_SOLST_Bit                             //7
}AF_info_byte1;

//PASS or _FAIL
enum {
	//BYTE 2 => byte này chỉ được Clear OK khi reset
	_INFO_PROBLEM_CURR_WRONG_Bit,				//0 => Khi Bật riêng từng tải và nếu thấy current bất thường thì bật bit này lên
	_INFO_PROBLEM_CURR_LEAKAGE0_Bit,            //1 => Chưa bật bất kỳ Relay nào nhưng đã có dòng điện => dính relay tổng hoặc dính chì ở AC PCB
	_INFO_PROBLEM_CURR_LEAKAGE1_Bit,            //2 => Mới bật Relay tổng(chưa bật Relay phụ) nhưng đã có dòng điện => Dính relay phụ hoặc dính chì
	_INFO_PROBLEM_CURR_ZERO_Bit,                //3 => Đã bật Relay tổng và relay phụ nhưng ko có dòng điện => Chưa cấp nguồn AC(chưa gắn AC Inlet) hoặc chưa gắn đủ tải(AC outlet)
	_INFO_PROBLEM_CURR_OVERLOAD_Bit,            //4 => Vượt quá dòng điện định mức cho phép 9.5A
	_INFO_PROBLEM_DISTEMP1_Bit,					//6 => disconect PTC thermal in Steamer container
	_INFO_PROBLEM_DISTEMP2_Bit,					//7 => disconect PTC thermal in AC Diode
	_INFO_PROBLEM_FAN_Bit                       //8 => Đang hoạt động nhưng Diode quá nóng (do Fan 24v ko hoạt động)
}AF_info_byte2;

//cac trang thai AF_ctrol de dieu khien AutoFill
typedef enum {
	_AF_CONTROL_NONE,		/*Trạng thái chưa điều khiển*/
	_AF_CONTROL_ENABLE,		/*Trạng thái sau khi vừa vặn Jetswt*/
	_AF_CONTROL_WAIT,		/*Trạng thái trạng thái đang Fill nước*/
	_AF_CONTROL_DISABLE,	/*Trạng thái tắt AutoFill*/
	_AF_CONTROL_ACTIVE,		/*Trạng thái On Jet & RGB của AutoFill*/
} AFCtrl_t;

/***************  Define Input AutoFill **********************/
typedef enum {
	_DRAIN_NORMAL,			/*Trạng thái chưa điều khiển*/
	_DRAIN_HOLD,           	/*Trạng thái nhấn giữ Sw và đang timeout 5 mins*/
	_DRAIN_HOLD_SSDRAIN,    /*Trạng thái nhấn giữ Sw ,sensor cạn nước và đang timeout 20s*/
	_DRAIN_HOLD_TIMEOUT		/*Trạng thái tăt Drain nhưng Swt vẫn còn nhấn giữ chưa trả về*/
} AF_DrainState_t;

typedef struct {
	__IO uint8_t 			Info_AF4[_BYTE_MAX];             	//Bien chua thong tin cac status cua AF4 ke ca Steamer
	uint8_t					Section;

	ActiveStatus			DrainSS_active;						//Drain Sensor phải Connected và chuyển từ High->Low ít nhất 1 lần sau khi khởi động
	ActiveStatus			Users_active;						//vặn Jetswt -> đầy nước -> bật lên 1 -> vặn Drain xa 10s ->tắt hết -> set về 0

	AFCtrl_t 				JetState;                       	//Cac trang thai DK by Jet swt
	AF_DrainState_t 		DrainState;                     	//Cac trang thai DK Drain

	struct SW_state 		_SW_Jet,
							_SW_Drain,
							_SW_Button;

	struct timer 			_timer_Auto_Off,                 	//sleep
							_timer_AdjLevel,                 	//timer xả thêm nước
							_timeout_Protect_Solenoid,         	//han che Solenoid On qua lau
							_timeout_Protect_Drain,            	//han che Drain On qua lau
							_timeout_AF_DRAIN_OffAll,        	//sau sau 10s phai nha nut moi tat RGB di , ko thi binh thuong
							_timer_Check_CapSS,              	//timer check sensor poll
							_timer_Deactive_DrainSS,			//sau khi bat solenoid, sau 30s ko thay Sensor xanh =>Sensor kha nang bi roi ra ngoai=>Deactive
							_timer_CheckRx;                  	//timer reset Rs232
} AF_Struct_t;

/* Global variables ---------------------------------------------------------*/
extern AF_Struct_t AF_BOX4;

void AF_Main_exe(void);
/*Hàm khởi tạo AutoFill*/
void AF_Init(AF_Struct_t *AF_Tmp);

/*Hàm điều khiển AutoFill*/
void AF_SectionOFF(AF_Struct_t *AF_Tmp);
void AF_SectionFINISH(AF_Struct_t *AF_Tmp);
void AF_SectionWAIT(AF_Struct_t *AF_Tmp);
void AF_CalibAllCapSS(void);
ResultStatus AF_CtrlRGB(CtrlRGB_TypeDef_t *RGB,
						AFCtrlRGB_TypeDef_t ON_OFF,
						AFSetRGB_Typedef ValCtrl);

/*Hàm Xử lý các nút nhấn của AutoFill*/
void AF_JetSW_progress(AF_Struct_t *AF_Tmp);
void AF_DrainSW_progress(AF_Struct_t *AF_Tmp);
void AF_ButtonSW_progress(AF_Struct_t *AF_Tmp);

/*Hàm Xử lý tất cả timeout của AutoFill*/
void AF_Timeout_progress(AF_Struct_t *AF_Tmp);

/*Hàm task manager của AutoFill*/
void AF_JetSwtCmd_Task(AF_Struct_t *AF_Tmp, LogicStatus Water_logic);
void AF_DrainSwtCmd_Task(AF_Struct_t *AF_Tmp,
						CheckStatus Dra_connect,
						LogicStatus Dra_logic);

#endif /* INC_AF_CONTROL_H_ */
