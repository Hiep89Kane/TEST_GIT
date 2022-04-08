#if !defined (RS232LL_H)
    #define RS232LL_H
/*Uncomment if you do not want Debug*/

/*Rename PutC & PutS */
//#define _RS232LL_PutChar                   uart4_putc
//#define _RS232LL_PutString                 uart4_puts

#define _RS232LL_PutChar(...)   			uart4_putc(__VA_ARGS__)
#define _RS232LL_PutString(...)   			uart4_puts(__VA_ARGS__)

/*Size of Rx Buffer*/
#define _RS232LL_RX_BUFF_SIZE              10
#define _RS232LL_RxLenData                 6

/*Define Device*/
#define _RS232_MASTER                      0
#define _RS232_SLAVE                       1
#define _RS232LL_I_am                      _RS232_SLAVE

/*=======================DEVICE ADDRESS=====================*/
//Range: 0x01 ...0x1D
#define _RS232LL_AddrDCS                    0x01
#define _RS232LL_AddrMsg                    0x02
#define _RS232LL_AddrAf4                    0x03
#define _RS232LL_AddrDt                     0x04
/*==========================================================*/

/*Define Addr*/
#define _RS232LL_AddrMe                    _RS232LL_AddrAf4
#define _RS232LL_AddrYou                   _RS232LL_AddrDCS

/*Start Byte & Stop Byte*/
#define _RS232LL_STX                        0x1E
#define _RS232LL_ETX                        0x1F
/*=======================MASSAGE CMD========================*/
    //Range: "01" ..."1F"
#if (_RS232LL_AddrMe==_RS232LL_AddrMsg || _RS232LL_AddrYou==_RS232LL_AddrMsg)
    //Control: "01" ..."0F"
    #define _RS232LL_MsgCmdCtrl_RaiseFw     "01"
    #define _RS232LL_MsgCmdCtrl_RaiseBw     "02"
    #define _RS232LL_MsgCmdCtrl_SlideFw     "03"
    #define _RS232LL_MsgCmdCtrl_SlideBw     "04"
    //Get Status: "10"..."1F"
#endif
/*==========================================================*/

/*=======================DAUGHTER CMD=======================*/
    //Range: "40" ..."5F"
#if (_RS232LL_AddrMe==_RS232LL_AddrDt || _RS232LL_AddrYou==_RS232LL_AddrDt)
    //Control: "40" ..."4F"
    #define _RS232LL_DtCmdCtrl_Service          0x40
    #define _RS232LL_DtCmdCtrl_ShowErr          0x41
    #define _RS232LL_DtCmdCtrl_Display          0x42
    #define _RS232LL_DtCmdCtrl_PutChar          0x43
    #define _RS232LL_DtCmdCtrl_Sleep            0x44
    //Get Status: "50"..."5F"
    #define _RS232LL_DtCmdGet_SttSensor         0x50
    #define _RS232LL_DtCmdGet_TempSensor        0x51
#endif
/*==========================================================*/

/*============================== BASE FUNCTIONS LIB =================================*/
void    	RS232LL_RxGetBuff(uint8_t data);
uint8_t  	RS232LL_RxCheck_Cmd_Data(uint8_t Receiver, uint8_t *RxCmd, uint8_t *RxData);
uint8_t   	RS232LL_RxCheckBuff(uint8_t *RxBuff, uint8_t Receiver, uint8_t Sender);
uint8_t   	RS232LL_TxReponse(uint8_t Device, uint8_t Check_Cmd_Data, uint8_t TxCmd, uint8_t TxData);
void    	RS232LL_RxClear(void);

#endif //RS232LL_H

/* [] END OF FILE */
