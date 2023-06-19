//================================================================
//================================================================
// CONTEC Linux DIO
// 割り込みサンプル
//               						    CONTEC Co.,Ltd.
//											Ver1.00
//================================================================
//================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/cdio.h"

//================================================================
// コールバック関数
//================================================================
void int_call_back(short id, short msg, long bit_no, long logic, void *param)
{
	char	up_down[100];
	//------------------------------------------
	// 割り込みメッセージの処理
	//------------------------------------------
	if (msg == DIOM_INTERRUPT) {
		if (logic == DIO_INT_RISE) {
			strcpy(up_down, "UP");
		} else if (logic == DIO_INT_FALL) {
			strcpy(up_down, "DOWN");
		} else {
			strcpy(up_down, "NONE");
		}
		fprintf(stdout, "Interrupt:id=%d, bit = %d, logic = %s: %s\n", (int)id, (int)bit_no, up_down, (char *)param);
	}
	//------------------------------------------
	// それ以外
	//------------------------------------------
	else {
		fprintf(stdout, "msg = %d\n", (int)msg);
	}
}

//================================================================
// main関数
//================================================================
int	main(int argc, char *argv[])
{
	short			bit_no;
	char			buf[256];
	long			lret;
	char			device_name[50];
	short			id;
	short			max_int = 4;
	//-------------------------------------
	// コマンドラインの解釈
	//-------------------------------------
	if (argc == 2) {
		strcpy(device_name, argv[1]);
	} else if (argc == 3) {
		strcpy(device_name, argv[1]);
		max_int = (short)atoi(argv[2]);
	} else {
		strcpy(device_name, "DIO000");
	}
	//-------------------------------------
	// 初期化
	//-------------------------------------
	lret = DioInit(device_name, &id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioInit", lret, buf);
		return -1;
	}
	//-------------------------------------
	// デジタルフィルタの設定
	//-------------------------------------
	lret = DioSetDigitalFilter(id, 20);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioSetDigitalFilter", lret, buf);
	}
	//-------------------------------------
	// 割り込みコールバックの設定
	//-------------------------------------
	lret = DioSetInterruptCallBackProc(id, (PDIO_INT_CALLBACK)int_call_back, (void *)"input 'q' + 'enter' to quit");
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioSetInterruptCallBackProc", lret, buf);
		return -1;
	}
	//-------------------------------------
	// 割り込みを有効にする
	//-------------------------------------
	for (bit_no=0; bit_no<max_int; bit_no++) {
		lret = DioSetInterruptEvent(id, bit_no, DIO_INT_RISE);
		if (lret != DIO_ERR_SUCCESS) {
			DioGetErrorString(lret, buf);
			fprintf(stderr, "%s=%ld:bit_no=%d:%s\n", "DioSetInterruptEvent", lret, (int)bit_no, buf);
			return -1;
		}
	}
	//-------------------------------------
	// 入力待ち
	//-------------------------------------
	while (1) {
		fprintf(stdout, "input 'q' + 'enter' to quit\n");
		fgets(buf, sizeof(buf), stdin);
		if (strstr(buf, "q") != NULL) {
			break;
		}
	}
	//-------------------------------------
	// 終了
	//-------------------------------------
	lret = DioExit(id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioExit", lret, buf);
		return -1;
	}
	return 0;
}


