/*
 * 1common.h
 *
 *  Created on: Feb 29, 2020
 *      Author: NGUYEN VAN HIEP
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include "stdint.h"
#include "stm32g0xx_hal.h"


/**@brief Macro for checking if an integer is a power of two.(bình phương của 2)
 *
 * @param[in]   A   Number to be tested.
 *
 * @return      true if value is power of two.
 * @return      false if value not power of two.
 */
#define IS_POWER_OF_TWO(A) 			(((A) != 0) && ((((A) - 1) & (A)) == 0))

//Phai co dau () cho ca Macro de khong nham thuat toan
/*Clear bit*/
#ifndef cbi
    #define cbi(port, bit)    		((port) &= ~(1 << (bit)))
#endif

/*Set bit*/
#ifndef sbi
    #define sbi(port, bit)    		((port) |=  (1 << (bit)))
#endif

/*Read bit*/
#ifndef rbi
    #define rbi(pin, bit)       	(((pin) >> (bit))&0x01)
#endif

/*Write bit*/
#ifndef wbi
    #define wbi(port, bit, val)		((val)? (sbi((port), (bit))) : (cbi((port), (bit))))
#endif

/*toggle bit*/
#ifndef tbi
	#define tbi(port, bit) 	   		((port) ^=  (1 << (bit)))
#endif

#ifndef half_LSB
	#define half_LSB(byte) 			(byte & 0b00001111)
#endif

#ifndef half_MSB
	#define half_MSB(byte) 			(byte & 0b11110000)>>4)
#endif

#define _TOGGLE_BYTE(BYTE)      	((BYTE) =(~BYTE) & 0x01)
#define _LIMIT(X,min,max)			(((X)>=(min)) && ((X)<=(max)))

#define COUNTOF(__BUFFER__)     	(sizeof(__BUFFER__) / sizeof(*(__BUFFER__))) // tra ve so phan tu trong 1 mang , phan tu do co the 1/2/3/4/ or x bytes

/*check if char is number macro*/
#define isnumber(num)       		((num>=48)&&(num<58))
/*check if char is hexa number macro*/
#define ishexnumber(num)      		(((num>=48)&&(num<58))||((num>=65)&&(num<71))||((num>=97)&&(num<103)))
/*check if char is uncased char macro*/
#define isUNCASEDchar(num)    		((num>='a')&&(num<='z'))
/*check if char is cased char macro*/
#define isCASEDchar(num)    		((num>='A')&&(num<='Z'))
/*check if char is readable char macro*/
#define isreadable(num)    			((num>=' ')&&(num<='~'))
/*watch dog reset*/

typedef enum{
	_FALSE,
	_TRUE = !_FALSE
}ResultStatus;

typedef enum{
	_FAIL,
	_PASS = !_FAIL
}CheckStatus;

typedef enum{
	_ERROR,
	_SUCCESS = !_ERROR
}ProcessStatus;

typedef enum{
	_CLEAR,
	_SET = !_CLEAR
}Flag_Status;

typedef enum{
	_LOW,
	_HIGH = !_LOW
}LogicStatus;

typedef enum{
	_DISABLE,
	_ENABLE = !_DISABLE
}FuncStatus;

typedef enum{
	_DEACTIVE,
	_ACTIVE = !_DEACTIVE
}ActiveStatus;

typedef enum{
	_NOT_BUSY,
	_BUSY = !_NOT_BUSY
}OperateStatus;

typedef enum{
	_NONE,		//0
	_RISING,	//1
	_FALLING,	//2
}Edge_ReturnStatus;

/*Define Reset Const*/
#define _ZERO		0
#define _NULL       0

LogicStatus 	Systick_ReadInput_poll(uint8_t InputIndex, GPIO_PinState Input, uint16_t LowSample_ms, uint16_t HighSample_ms);
void 			delay_ms(uint32_t ms);

void    		uncase_to_case(char *str);
char    		strchecksum(char *str);
uint8_t 		Str2HexChar(char *str, uint8_t *HexChar);
void    		HexChar2Str(char *str, uint8_t HexChar);
uint32_t  		Str2Num(char *str);
void    		Num2Str(char *str, uint32_t number);

void 			lstrncpy(char *str1,char *str2,unsigned int n);
void 			lstrncat(char *str1,char *str2,unsigned int n);

unsigned long 	string2ulong(char *str);
unsigned int 	string2uint(char *str);
unsigned char 	string2hexchar(char *str);
char 			string2charxx(char *str);

Edge_ReturnStatus Get_Edge(uint8_t Pin_stt);
#endif /* INC_COMMON_H_ */
