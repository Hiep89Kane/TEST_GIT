///////////////////////////////////////////////////////////////////////////////////
//CHUONG TRINH GOP CHUOI DU LIEU TU DONG
//		Design: Cao Van Tuan
//		Mail: caovantuan1792@gmail.com
//////////////////////////////////////////////////////////////////////////////////

/**
 * Giải thích như thế này :
 * Tồn tại 2 buffer với buffer1=[0][100] và buffer2=[1][100] , 
 * id_read : để biết là dữ liệu đang ở đâu để chờ ghi vào bộ đệm , count[id_read] là vị trí byte tiếp theo
 * - nếu id_read nhận đủ 1 string (\n , \r , hết timeout khoảng cách giữa 1 byte) => thì id_read++ để chuyển sang buffer khác
 * id_write : nếu id_write != id_read tức là có 1 bộ đệm đã nhận đủ dữ liệu và chương trình nhảy vào check , sau đó id_write++ để đuổi theo (id_read)
 * - Nếu (id_read) ko có dữ liệu mới thì (id_write==id_read) chương trình ko check nữa
 * - Nếu (id_read) có dữ liệu mới thì (id_write!=id_read) chương trình nhảy vào check tiếp và id_write++ ...
 * **/


#include "get_string.h"

//ghi tuần tự byte vào 2 bộ đệm
void get_string_get_input(get_string_str *sGS, char byte){	
	sGS->string[sGS->id_read][sGS->count[sGS->id_read]++] = byte; //ghi byte nhận được vào buffer Read
	
	//Chỉ kiểm tra khi đã nhận ít nhất 2 byte
	//nếu có ký tự \n\r(thông thường là tập lệnh AT) thì kết thúc chuỗi nhận , hoặc hết timeout	
	if(sGS->count[sGS->id_read] > 2 \
	&& sGS->string[sGS->id_read][sGS->count[sGS->id_read]-1] == 0x0A \
	&& sGS->string[sGS->id_read][sGS->count[sGS->id_read]-2] == 0x0D){
		//=> kết thúc chuỗi read 1=> chuyển qua chuỗi read thứ 2 => tiếp tục quay về read 1 =>.... (và reset => count trong chuỗi đó về vị trí 0)
		if(++sGS->id_read >= MAX_STRING)
			sGS->id_read = 0;
		sGS->count[sGS->id_read] = 0;
		timer_stop(&sGS->timeout);
		return;
	}
	//tràn bộ đệm
	if(sGS->count[sGS->id_read] == MAX_LEN_STRING)
		sGS->count[sGS->id_read] = MAX_LEN_STRING - 1;
	//timeout của byte tiếp theo => nếu quá timeout này xem như đã nhận đủ	
	timer_set(&sGS->timeout, TIMEOUT_STRING);
}

void get_string_init_notify(get_string_str *sGS, void (*void_get_string_notify)(char *string, uint16_t size)){
	sGS->void_notify = void_get_string_notify;
	sGS->flag_notify = 1; //đã init hàm void_get_string_notify
}

void get_string_loop_manage(get_string_str *sGS){
	//Khi id_write != id_read => nghĩa đã có bộ đệm có dữ liệu
	if(sGS->id_write != sGS->id_read){
		sGS->string[sGS->id_write][sGS->count[sGS->id_write]] = 0;
		//đã init hàm void_notify => gọi hàm này để xử lí (tham số là sGS->string[sGS->id_write])
		if(sGS->flag_notify == 1){
			sGS->void_notify(sGS->string[sGS->id_write], sGS->count[sGS->id_write]);
		}
		//xử lí xong thì ++ => lúc này : id_write == id_read nếu ko nhận đc chuỗi mới
		if(++sGS->id_write >= MAX_STRING)
			sGS->id_write = 0;
	}

	//cứ hết timeout 1 byte thì nhảy qua buffer khác
	if(timer_expired(&sGS->timeout)){
		if(++sGS->id_read >= MAX_STRING)
			sGS->id_read = 0;
		sGS->count[sGS->id_read] = 0;
		timer_stop(&sGS->timeout);
	}
}

//THE END
