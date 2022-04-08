#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "myHeader.h"

//set timer delay => sau timer này sẽ lưu Flash mới
#define Flash_SetTimer()				timer_set(&_timeout_WriteFlash, 2*CLOCK_SECOND)
#define Flash_SetTimer_SaveReport()		timer_set(&_timeout_WriteFlash, 5*CLOCK_MINUTE) //5 phut
#define GetPage(Addr) 	 				((Addr - FLASH_BASE) / FLASH_PAGE_SIZE)      	//trả về vị trí Page của 1 địa chỉ bất kỳ

//FLASH_PAGE_SIZE = 2k bytes

//luu y : stm32g070RB : co 64 pages , moi page 2k byte ,FLASH_PAGE_SIZE :  0x00000800U = 2048 bytes ~ 2kb
#define ADDR_FLASH_PAGE_63   	0x0801F800   	/* Start @ of user Flash area =>  last byte of page 63 is 0x0801 FFFF */

#define FLASH_RETRY_PROGRAM		3 				//toi da 3 lan retry

typedef enum {
	_IDF_DWORD_MAC_ADDRESS=0,					//0 =>Mã định danh MAC cua bộ AutoFill 4
	_IDF_DWORD_AF,                              //1
	_IDF_DWORD_ST,                              //2
	_IDF_DWORD_ST_REPORT,						//3 //=> lưu giữ số liệu của STEAMResponse_TypeDef

	_IDF_DWORD_USER_MAX
} DOUBLE_WORD_NUM;

typedef enum {
	//for AF
	_IDF_WTER_LEVEL,                            //0
	_IDF_RGB_MASTERCTRL,                        // 0:full color , 1->7 : single color => Set mau

	_IDF_AF_REVERSION = 7,						//Lưu trữ thông tin về reversion để update firware sau này
	_IDF_AF_MAX = 8                             //8
} DOUBLE_WORD_AF_position;

typedef enum {
	//for Steam
	_IDF_ST_TEMPTHRES,                          //0
	_IDF_ST_TIMESERVICE,                        //1
	_IDF_ST_BLINKSOL,                           //2
	_IDF_ST_ENSAFETY,                           //3
	_IDF_ST_SPOTLIGHT,							//4
	_IDF_ST_POWER_TRIAC,						//5
												//6
												//7
	_IDF_ST_MAX = 8                             //8
} DOUBLE_WORD_ST_position;

/* Global variables ---------------------------------------------------------*/
extern uint64_t		AF4_MAC_u64_Flash;			//chứa dữ liệu 8 bytes địa chỉ MAC ADRR => nằm ở sector đầu tiên của Page cuối cùng

extern uint8_t		AF_Flash[_IDF_AF_MAX],		//Chứa dữ liệu Epprom của AutoFill
					ST_Flash[_IDF_ST_MAX],		//Chứa dữ liệu Epprom của Steamer
					ST_Report_Flash[8];			//Chứa dữ liệu thông kê hoạt động của Steamer

extern struct timer _timeout_WriteFlash;	//Han che ghi flash lien tuc

void 		Flash_Main_exe(void);

//tra ve so thu tu Page vi du : 0 ->63
ResultStatus Erase_Flash(uint32_t ADR_StartPage, uint32_t numPages);

uint8_t 	Flash_ReadByte(uint32_t ADR);
uint32_t 	Flash_ReadWord(uint32_t ADR);
uint64_t 	Flash_ReadDWord(uint32_t ADR);

ResultStatus Flash_Write_DWord(uint64_t Data_In[_IDF_DWORD_USER_MAX], uint32_t ADR_StartPage);

//su dung cho G0 => vi moi lan ghi là 64 bit = FLASH_TYPEPROGRAM_DOUBLEWORD
uint64_t 	Combine_to_DWord(uint8_t *Data_Arr);
void 		Separate_from_DWord(uint8_t *Data_Out, uint64_t DWord);

//Phải lưu Flash ở đây ,chống trường hợp lưu Flash liên tục
void 		Flash_AF4_SaveAll(uint64_t *MAC_flash_u64, uint64_t AF_flash_u64, uint64_t ST_flash_u64, uint64_t ST_report_flash_u64);
#endif /* INC_FLASH_H_ */
