#include "myHeader.h"

//USART Receiver buffer
static uint8_t 		RS232LL_RxBuff[_RS232LL_RX_BUFF_SIZE];
static uint8_t	 	RS232LL_RxCmdStr[3], RS232LL_RxDataStr[3];
static uint8_t 		RS232LL_RxCmd, RS232LL_RxData;
static uint8_t 		RS232LL_RxWrIndex=0;
static uint8_t 		RS232LL_RxCnt=0;
//USART Tranmister buffer
static uint8_t 		RS232LL_TxCmdStr[3], RS232LL_TxDataStr[3];

//This flag is set on USART Receiver buffer overflow
uint8_t 		RS232LL_RxOverBuff=_DEACTIVE;
uint8_t 		RS232LL_RxEnable=_DEACTIVE;

static uint8_t  RS232LL_Str2HexChar(uint8_t *str, uint8_t *HexChar);
static void    	RS232LL_HexChar2Str(uint8_t *str, uint8_t HexChar);

/*************************************************************
*Name: RS232LL_RxCheck_Cmd_Data
 *Chuc nang: Kiem tra Command_code va Data, neu dung thi ghi du
            lieu vao bien toan cuc, sai thi khong ghi
 *Cach dung: dat trong ham Main
            RS232LL_RxCheck_Cmd_Data(_RS232LL_AddrMe,&Cmd,&Data)
 *Tham so truyen: Receiver, *RxCmd, *RxData
 *Gia tri thay doi: *RxCmd, *RxData. nguoi dung can khai bao
                    hai bien toan cuc, day chinh la du lieu Slave
                    lay duoc khi khung truyen PASS
 *Tra ve: 1-True   0-False
*/
uint8_t RS232LL_RxCheck_Cmd_Data(uint8_t Receiver, uint8_t *RxCmd, uint8_t *RxData){
    if (RS232LL_RxCheckBuff(RS232LL_RxBuff,_RS232LL_AddrMe,_RS232LL_AddrYou)==0) return 0;
    if (Receiver==_RS232LL_AddrMsg){
        if (RS232LL_RxCmd>0x00 && RS232LL_RxCmd<=0x1F) *RxCmd=RS232LL_RxCmd;
        else return 0;
    }
    else if (Receiver==_RS232LL_AddrAf4){
        if (RS232LL_RxCmd>=0x20 && RS232LL_RxCmd<=0x3F) *RxCmd=RS232LL_RxCmd;
        else return 0;
    }
    else if (Receiver==_RS232LL_AddrDt){
        if (RS232LL_RxCmd>=0x40 && RS232LL_RxCmd<=0x5F) *RxCmd=RS232LL_RxCmd;
        else return 0;
    }
    if (*RxCmd>0x00) *RxData=RS232LL_RxData;

    return 1;
}

/*************************************************************
*Name: RS232LL_RxClear
 *Chuc nang: Xoa Rx_Buffer va cac tham bien lien quan
 *Cach dung: dat trong ham RS232LL_RxCheckBuff
 *Tham so truyen: khong
 *Gia tri thay doi: RS232LL_RxBuff[] va cac tham bien khac
 *Tra ve: khong
*/
 void RS232LL_RxClear(void){
    //memset(RS232LL_RxBuff,0x00,sizeof(RS232LL_RxBuff));
    RS232LL_RxBuff[0] = '\0';
    RS232LL_RxCmdStr[0]='\0';
    RS232LL_RxDataStr[0]='\0';
    RS232LL_RxCnt=_ZERO;
    RS232LL_RxWrIndex=_ZERO;
    RS232LL_RxOverBuff =_DEACTIVE;
    RS232LL_RxEnable = _DEACTIVE;
}

/*************************************************************
*Name: RS232LL_Str2HexChar
 *Chuc nang: Chuyen 2 ki tu bieu dien kieu HEX thanh interger
 *Cach dung: truyen string[2byte] va con tro cua tham bien
 *Tham so truyen: *str , HexChar
 *Gia tri thay doi: *HexChar
 *Tra ve: 1-True,    0-False
*/
static uint8_t RS232LL_Str2HexChar(uint8_t *str, uint8_t *HexChar){
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
*Name: RS232LL_HexChar2Str
 *Chuc nang: Chuyen gia tri Hex Char thanh String, VD: 0xAB thanh "AB"
 *Cach dung: truyen string[2byte] va tham bien interger can chuyen
 *Tham so truyen: *str , HexChar
 *Gia tri thay doi: *str
 *Tra ve: khong
*/
static void RS232LL_HexChar2Str(uint8_t *str, uint8_t HexChar){
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

/*************************************************************************************************************************
*Name: RS232LL_RxGetBuff
 *Chuc nang: Lay bo dem 6 byte Rx voi yeu cau dinh dang STX-[6 byte data]-ETX
 *Cach dung: dat trong ham ngat Rx
 *Tham so truyen: data interrupt
 *Gia tri thay doi: RS232LL_RxOverBuff, RS232LL_RxBuff
 *Tra ve: khong co
*/
void RS232LL_RxGetBuff(uint8_t data){
    if(RS232LL_RxEnable==_DEACTIVE){
        if (data==_RS232LL_STX){
            RS232LL_RxClear();
            RS232LL_RxEnable=_ACTIVE;
        }
    }
    else{
        if (data==_RS232LL_STX){
            RS232LL_RxBuff[0]='\0';      	 				//Xoa bo dem
            RS232LL_RxWrIndex=_ZERO;
            RS232LL_RxCnt=_ZERO;
        }
        else if(data==_RS232LL_ETX){
            if (RS232LL_RxWrIndex!=_RS232LL_RxLenData){
                RS232LL_RxClear();
                return;
            }
            RS232LL_RxBuff[RS232LL_RxWrIndex]='\0';       	//ket thuc chuoi buffer
            RS232LL_RxOverBuff=_ACTIVE;                 	//Bat co len nhan da du du lieu
            RS232LL_RxEnable = _DEACTIVE;
        }
        else{
            RS232LL_RxBuff[RS232LL_RxWrIndex++]=data;
            if (RS232LL_RxWrIndex == _RS232LL_RX_BUFF_SIZE) RS232LL_RxWrIndex=_ZERO;
            if (++RS232LL_RxCnt == _RS232LL_RX_BUFF_SIZE){
                RS232LL_RxCnt=_ZERO;
                RS232LL_RxOverBuff=_ACTIVE;
            }
        }
    }
}

/*************************************************************
*Name: RS232LL_RxCheckBuff
 *Chuc nang: Kiem tra Rx_Buffer va Xoa Rx_Buffer
 *Cach dung: dat trong ham RS232LL_RxCheck_Cmd_Data
 *Tham so truyen: RxBuff, Receiver, Sender
 *Gia tri thay doi: RS232LL_RxCmdStr, RS232LL_RxDataStr, RS232LL_RxCmd, RS232LL_RxData
 *Tra ve: 1-True   0-False
*/
uint8_t RS232LL_RxCheckBuff(uint8_t *RxBuff, uint8_t Receiver, uint8_t Sender){
    uint8_t Check_HexChar;
    if (RS232LL_RxOverBuff==_DEACTIVE)  return 0;

//    #ifdef _RS232LL_DEBUG
//        _RS232LL_PutString(RxBuff);
//    #endif

    if ((RxBuff[0]!=Receiver)||(RxBuff[1]!=Sender)){
        RS232LL_RxClear();
        return 0;
    }

    RS232LL_RxCmdStr[0]=RxBuff[2];
    RS232LL_RxCmdStr[1]=RxBuff[3];
    RS232LL_RxCmdStr[2]='\0';
    Check_HexChar=RS232LL_Str2HexChar(RS232LL_RxCmdStr,&RS232LL_RxCmd);
    if (Check_HexChar==0){
        RS232LL_RxClear();
        return 0;
    }

    RS232LL_RxDataStr[0]=RxBuff[4];
    RS232LL_RxDataStr[1]=RxBuff[5];
    RS232LL_RxDataStr[2]='\0';
    Check_HexChar=RS232LL_Str2HexChar(RS232LL_RxDataStr,&RS232LL_RxData);
    if (Check_HexChar==0){
        RS232LL_RxClear();
        return 0;
    }

//    #ifdef _RS232LL_DEBUG
//        _RS232LL_PutString(RS232LL_RxCmdStr);
//        _RS232LL_PutString(RS232LL_RxDataStr);
//    #endif

    RS232LL_RxClear();
    return 1;
}



/*************************************************************
*Name: RS232LL_TxReponse
 *Chuc nang: Reponse lai Master neu da nhan dung du lieu
 *Cach dung: dat trong ham Main
 *Tham so truyen: Device, Check_Cmd_Data, TxCmd, TxData
 *Gia tri thay doi: RS232LL_TxCmdStr[], RS232LL_TxDataStr[]
 *Tra ve: 1-True   0-False
*/
uint8_t RS232LL_TxReponse(uint8_t Device, uint8_t Check_Cmd_Data, uint8_t TxCmd, uint8_t TxData){
    if (Device==_RS232_MASTER)  return 0;
    if (Check_Cmd_Data==0)      return 0;

//    /*Xu ly de HEX char du 2 chu cai */
//    if (TxCmd>0x0F)     sprintf(RS232LL_TxCmdStr,"%X",TxCmd);
//    else                sprintf(RS232LL_TxCmdStr,"0%X",TxCmd);
//
//    if (TxData>0x0F)    sprintf(RS232LL_TxDataStr,"%X",TxData);
//    else                sprintf(RS232LL_TxDataStr,"0%X",TxData);
    RS232LL_HexChar2Str(RS232LL_TxCmdStr,TxCmd);
    RS232LL_HexChar2Str(RS232LL_TxDataStr,TxData);

    _RS232LL_PutChar(_RS232LL_STX);
    _RS232LL_PutChar(_RS232LL_AddrYou);
    _RS232LL_PutChar(_RS232LL_AddrMe);
	_RS232LL_PutChar(RS232LL_TxCmdStr[0]);
	_RS232LL_PutChar(RS232LL_TxCmdStr[1]);
    _RS232LL_PutChar(RS232LL_TxDataStr[0]);
	_RS232LL_PutChar(RS232LL_TxDataStr[1]);
    _RS232LL_PutChar(_RS232LL_ETX);

    return 1;
}

/* [] END OF FILE */
