#include "myHeader.h"

#define WRD()	HAL_IWDG_Refresh(&hiwdg)

//==================================== HAM Input Co ban ==================================================================
LogicStatus Systick_ReadInput_poll(uint8_t InputIndex, GPIO_PinState Input, uint16_t LowSample_ms, uint16_t HighSample_ms){
	static uint16_t Input_HighCnt[_Input_Total]={0};
	static uint16_t Input_LowCnt[_Input_Total]={0};
	static LogicStatus ReturnVal[_Input_Total]={_HIGH};
	if (Input == GPIO_PIN_RESET){
		Input_HighCnt[InputIndex]=0;
		if (++Input_LowCnt[InputIndex] >= LowSample_ms){
			Input_LowCnt[InputIndex]=0;
			ReturnVal[InputIndex]=_LOW;
		}
	}else if (Input == GPIO_PIN_SET){
		Input_LowCnt[InputIndex]=0;
		if (++Input_HighCnt[InputIndex] >= HighSample_ms){
			Input_HighCnt[InputIndex]=0;
			ReturnVal[InputIndex]=_HIGH;
		}
	}
	return ReturnVal[InputIndex];
}

void 	delay_ms(uint32_t ms){
	WRD();
	HAL_Delay(ms);
}

void uncase_to_case(char *str)
{
  unsigned int i;
  for(i=0;str[i]!=0;i++)if(str[i]>='a' && str[i]<='z')str[i] -= 32;
}

char strchecksum(char *str)
{
  unsigned int i;
  uint8_t CheckSum=0;
  for(i=0;str[i]!=0;i++){
    CheckSum=CheckSum^str[i];
  }
  return CheckSum;
}

/*************************************************************
*Name: Str2HexChar
 *Chuc nang: Chuyen 2 ki tu bieu dien kieu HEX thanh interger
 *Cach dung: truyen string[2byte] va con tro cua tham bien
 *Tham so truyen: *str , HexChar
 *Gia tri thay doi: *HexChar
 *Tra ve: 1-True,    0-False
*/
uint8_t Str2HexChar(char *str, uint8_t *HexChar){
  uint8_t base;
  if ((str[0]>=97)&&(str[0]<103))base=97-10;           //a-f
  else if ((str[0]>=65)&&(str[0]<71))base=65-10;       //A-F
  else if ((str[0]>=48)&&(str[0]<58))base=48;          //0-9
  else return 0;            //err hex char
  *HexChar=16*(str[0]-base);
  if ((str[1]>=97)&&(str[1]<103))base=97-10;
  else if ((str[1]>=65)&&(str[1]<71))base=65-10;
  else if ((str[1]>=48)&&(str[1]<58))base=48;
  else return 0;            //err hex char
  *HexChar+=(str[1]-base);
  return 1;
}

/*************************************************************
*Name: HexChar2Str
 *Chuc nang: Chuyen gia tri Hex Char thanh String, VD: 0xAB thanh "AB"
 *Cach dung: truyen string[2byte] va tham bien interger can chuyen
 *Tham so truyen: *str , HexChar
 *Gia tri thay doi: *str
 *Tra ve: khong
*/
void HexChar2Str(char *str, uint8_t HexChar){
    uint8_t Shift_Byte=0, base;
    Shift_Byte=(HexChar>>4)&0x0F;
    if (Shift_Byte>=0 && Shift_Byte<=9) base='0'-0;
    else                                base='A'-0x0A;
    str[0]=Shift_Byte+base;
    Shift_Byte=HexChar&0x0F;
    if (Shift_Byte>=0 && Shift_Byte<=9) base='0'-0;
    else                                base='A'-0x0A;
    str[1]=Shift_Byte+base;
}
/*************************************************************
*Name: Str2Num
 *Chuc nang: Chuyen gia tri chuoi thanh so tuong ung, VD: "109" thanh 109 , gioi han 10 ki tu
*/
uint32_t Str2Num(char *str){
    uint8_t i;
    uint32_t num=0;
    for(i=0;str[i]>='0' && str[i]<='9' && i<10;i++){
        num*=10;
        num+=(str[i]-'0');
    }
    return num;
}
/*************************************************************
*Name: Num2Str
 *Chuc nang: Chuyen gia tri so thanh chuoi tuong ung, VD: 109 thanh "109"
*/
void Num2Str(char *str, uint32_t number){
	char a[10];
	uint8_t i,j;
	for(i=0;i<10;i++){
		if(number>0){
			a[i]=number%10 + '0';
			number/=10;
		}else break;
	}
	if(i==0){
		str[0]='0';
		str[1]='\0';
		return;
	}
	for(j=0;j<i;j++){
		str[j]=a[i-j-1];
	}
	str[i]='\0';
}

void lstrncpy (char *str1,char *str2,unsigned int n)
{
  unsigned int i=0;
  do
  {
    str1[i]=str2[i];
  }while((str2[i]!=0) && (++i<n));
  str1[i]=0;
}

void lstrncat (char *str1,char *str2,unsigned int n)
{
  unsigned int i=0;
  for (;*str1!=0;str1++);
  do
  {
    str1[i]=str2[i];
  }while((str2[i]!=0) && (++i<n));
  str1[i]=0;
}


Edge_ReturnStatus Get_Edge(uint8_t Pin_stt){
	uint8_t result;
	static uint8_t Pin_stt_old;

	if(Pin_stt == Pin_stt_old) return _NONE;

	if(Pin_stt == 0) result = _FALLING;
	else result = _RISING;

	Pin_stt_old = Pin_stt;
	return result;
}
