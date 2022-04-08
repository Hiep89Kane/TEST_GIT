#include "myHeader.h"
#include "stdlib.h"

uint64_t AF4_MAC_u64_Flash;					//chứa dữ liệu 8 bytes địa chỉ MAC ADRR => nằm ở sector đầu tiên của Page cuối cùng

uint8_t	AF_Flash[_IDF_AF_MAX],				//Chứa dữ liệu Epprom của AutoFill
		ST_Flash[_IDF_ST_MAX],				//Chứa dữ liệu Epprom của Steamer
		ST_Report_Flash[8];

struct timer _timeout_WriteFlash;			//Han che ghi flash lien tuc

/*Variable used for Erase procedure*/
static uint32_t PageError = 0;
//==========================================================================================================================================

ResultStatus Erase_Flash(uint32_t ADR_StartPage, uint32_t numPages){
	FLASH_EraseInitTypeDef 	EraseInitStruct;

	HAL_FLASH_Unlock();
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Page = GetPage(ADR_StartPage);
	EraseInitStruct.NbPages = numPages; //1 page

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_OK){
																									//debug_msg("\n\rXoa Page %X OK",ADR_StartPage);
		return _TRUE;
	}else return _FALSE;
}

uint8_t Flash_ReadByte(uint32_t ADR){
	uint8_t Value;
	Value = *(__IO uint8_t *)ADR;
	return Value;
}

uint32_t Flash_ReadWord(uint32_t ADR){
	uint32_t Value;
	Value = *(__IO uint32_t *)ADR;
	return Value;
}

uint64_t Flash_ReadDWord(uint32_t ADR){
	uint64_t Value;
	Value = *(__IO uint64_t *)ADR;
	return Value;
}


// example : uint8_t data[8] ={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; => 0x0706050403020100
uint64_t Combine_to_DWord(uint8_t *Data_Arr){
	uint64_t Data_tmp;
	uint8_t	i =0;

	Data_tmp = 0;

	for(i=0; i<8; i++) Data_tmp += ((uint64_t)Data_Arr[i])<<(8*i);

	return Data_tmp;
}

// example :  0x0706050403020100 => uint8_t data[8] ={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
void Separate_from_DWord(uint8_t *Data_Out, uint64_t DWord){

	for(uint8_t i=7; i>=0; i--){
		Data_Out[i] = (DWord >>(8*i));
		if(i==0)return;
	}
}

ResultStatus Flash_Write_DWord(uint64_t Data_In[_IDF_DWORD_USER_MAX], uint32_t ADR_StartPage){
		FLASH_EraseInitTypeDef 	EraseInitStruct;
		uint64_t 				Data_Buff[_IDF_DWORD_USER_MAX], Data_Check[_IDF_DWORD_USER_MAX]; 	//vi dong G0 bat luu 1 lan 64 bit
		uint8_t					retry[_IDF_DWORD_USER_MAX], i;

		HAL_FLASH_Unlock();
		//Erase this page
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Page        = GetPage(ADR_StartPage);	//=63
		EraseInitStruct.NbPages     = 1;
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){ /*debug_msg("\n\rErase OK");*/}

		//Write data
		for(i=0; i<_IDF_DWORD_USER_MAX; i++){
			Data_Buff[i] = Data_In[i];
			retry[i] = FLASH_RETRY_PROGRAM;
			do{
				if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ADR_StartPage + 8*i, Data_Buff[i])== HAL_OK){
					//debug_msg("\n\rGhi Data_In[%u]= %x !",i, Data_Buff[i]);
				}
				Data_Check[i] = Flash_ReadDWord(ADR_StartPage + 8*i);
				retry[i]--;
			}while(Data_Check[i] !=  Data_Buff[i]  && (retry[i] != 0));
		}

		HAL_FLASH_Lock();

		for(i=0; i<_IDF_DWORD_USER_MAX; i++){
			if(retry[i]==0) return _FALSE;
		}
		return _TRUE;
}

void Flash_AF4_SaveAll(uint64_t *MAC_flash_u64, uint64_t AF_flash_u64, uint64_t ST_flash_u64, uint64_t ST_report_flash_u64){
	uint8_t i;
	uint64_t u64_tmp_arr[_IDF_DWORD_USER_MAX];
	uint64_t this_year,this_month,random1,random2;

	if(timer_expired(&_timeout_WriteFlash)){
		timer_stop(&_timeout_WriteFlash);

		//gán địa chỉ MAC lần đầu
		if((*MAC_flash_u64 == (0xFFFFFFFFFFFFFFFF)) || (*MAC_flash_u64 == 0)){
			this_year = _THIS_YEAR; //2021
			this_month = _THIS_MONTH;//tháng 09
			random1 = _THIS_MONTH*(_USER_DEFINE_TIM_RGB.Instance->CNT);
			random2 = random1*_THIS_YEAR*HAL_GetTick();

			*MAC_flash_u64 = (this_year<<56) + (this_month<<48) + ((random1<<32) & (0x0000FFFFFFFFFFFF)) + random2;

		}

		u64_tmp_arr[_IDF_DWORD_MAC_ADDRESS] = *MAC_flash_u64;
		u64_tmp_arr[_IDF_DWORD_AF] 			= AF_flash_u64;
		u64_tmp_arr[_IDF_DWORD_ST] 			= ST_flash_u64;
		u64_tmp_arr[_IDF_DWORD_ST_REPORT] 	= ST_report_flash_u64;

		Flash_Write_DWord(u64_tmp_arr, ADDR_FLASH_PAGE_63);
																										debug_msg("\nMAC=");
																										for(i=1; i<=8 ; i++){
																											debug_msg("%x ",(uint8_t)(u64_tmp_arr[_IDF_DWORD_MAC_ADDRESS] >>(64-i*8)));
																										}

																										debug_msg("\nAF=");
																										for(i=1; i<=8 ; i++){
																											debug_msg("%x ",(uint8_t)(u64_tmp_arr[_IDF_DWORD_AF] >>(64-i*8)));
																										}

																										debug_msg("\nST=");
																										for(i=1; i<=8 ; i++){
																											debug_msg("%x ",(uint8_t)(u64_tmp_arr[_IDF_DWORD_ST]>>(64-i*8)));
																										}

																										debug_msg("\nRP=");
																										for(i=1; i<=8 ; i++){
																											debug_msg("%x ",(uint8_t)(u64_tmp_arr[_IDF_DWORD_ST_REPORT]>>(64-i*8)));
																										}
	}
}

void Flash_Main_exe (void){

	  Flash_AF4_SaveAll( &AF4_MAC_u64_Flash,
			  	  	  	 Combine_to_DWord(AF_Flash),
						 Combine_to_DWord(ST_Flash),
						 Combine_to_DWord(ST_Report_Flash));
}

