//================================================================
//================================================================
// CONTEC Linux DIO
// 入出力サンプル
//               						    CONTEC Co.,Ltd.
//											Ver1.00
//================================================================
//================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/cdio.h"

//================================================================
// コマンド定義
//================================================================
#define	COMMAND_ERROR		0	// エラー
#define	COMMAND_INP_PORT	1	// 1ポート入力
#define	COMMAND_INP_BIT		2	// 1ビット入力
#define	COMMAND_OUT_PORT	3	// 1ポート出力
#define	COMMAND_OUT_BIT		4	// 1ビット出力
#define	COMMAND_ECHO_PORT	5	// 1ポートエコーバック
#define	COMMAND_ECHO_BIT	6	// 1ビットエコーバック
#define	COMMAND_QUIT		7	// 終了

//================================================================
// main関数
//================================================================
int	main(int argc, char *argv[])
{
	short			port_no;
	short			bit_no;
	unsigned char	data;
	char			buf[256];
	int				command;
	long			lret;
	char			device_name[50];
	short			id;
	int				itmp;
	//-------------------------------------
	// コマンドラインの解釈
	//-------------------------------------
	if (argc == 2) {
		strcpy(device_name, argv[1]);
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
	// 入力待ちループ
	//-------------------------------------
	while(1) {
		//-------------------------------------
		// コマンドの表示
		//-------------------------------------
		fprintf(stdout, "ip : port input\n");
		fprintf(stdout, "ib : bit  input\n");
		fprintf(stdout, "op : port output\n");
		fprintf(stdout, "ob : bit  output\n");
		fprintf(stdout, "ep : port echoback\n");
		fprintf(stdout, "eb : bit  echoback\n");
		fprintf(stdout, "q  : quit\n");
		fprintf(stdout, "input command:");
		//-------------------------------------
		// 入力待ち
		//-------------------------------------
		fgets(buf, sizeof(buf), stdin);
		//-------------------------------------
		// コマンドの識別
		//-------------------------------------
		command = COMMAND_ERROR;
		if (strstr(buf, "ip") != NULL) {
			command = COMMAND_INP_PORT;		// 1ポート入力
		}
		if (strstr(buf, "ib") != NULL) {
			command = COMMAND_INP_BIT;		// 1ビット入力
		}
		if (strstr(buf, "op") != NULL) {
			command = COMMAND_OUT_PORT;		// 1ポート出力
		}
		if (strstr(buf, "ob") != NULL) {
			command = COMMAND_OUT_BIT;		// 1ビット出力
		}
		if (strstr(buf, "ep") != NULL) {
			command = COMMAND_ECHO_PORT;	// 1ポートエコーバック
		}
		if (strstr(buf, "eb") != NULL) {
			command = COMMAND_ECHO_BIT;		// 1ビットエコーバック
		}
		if (strstr(buf, "q") != NULL) {
			command = COMMAND_QUIT;			// 終了
		}
		//-------------------------------------
		// ポート番号、ビット番号の入力
		//-------------------------------------
		switch (command) {
		case COMMAND_INP_PORT:		// 1ポート入力
		case COMMAND_OUT_PORT:		// 1ポート出力
		case COMMAND_ECHO_PORT:		// 1ポートエコーバック
			fprintf(stdout, "input port number:");
			fgets(buf, sizeof(buf), stdin);
			port_no = (short)atoi(buf);
			break;
		case COMMAND_INP_BIT:		// 1ビット入力
		case COMMAND_OUT_BIT:		// 1ビット出力
		case COMMAND_ECHO_BIT:		// 1ビットエコーバック
			fprintf(stdout, "input bit number:");
			fgets(buf, sizeof(buf), stdin);
			bit_no = (short)atoi(buf);
			break;
		default:
			break;
		}
		//-------------------------------------
		// データの入力
		//-------------------------------------
		switch (command) {
		case COMMAND_OUT_PORT:		// 1ポート出力
		case COMMAND_OUT_BIT:		// 1ビット出力
			fprintf(stdout, "input data (hex):");
			fgets(buf, sizeof(buf), stdin);
			itmp = 0;
			sscanf(buf, "%x", &itmp);
			data = (unsigned char)(itmp & 0xff);
			break;
		default:
			break;
		}
		//-------------------------------------
		// コマンドの実行と結果の表示
		//-------------------------------------
		switch (command) {
		case COMMAND_INP_PORT:		// 1ポート入力
			lret = DioInpByte(id, port_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioInpByte", lret, buf);
			}
			break;
		case COMMAND_INP_BIT:		// 1ビット入力
			lret = DioInpBit(id, bit_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioInpBit", lret, buf);
			}
			break;
		case COMMAND_OUT_PORT:		// 1ポート出力
			lret = DioOutByte(id, port_no, data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioOutByte", lret, buf);
			}
			break;
		case COMMAND_OUT_BIT:		// 1ビット出力
			lret = DioOutBit(id, bit_no, data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioOutBit", lret, buf);
			}
			break;
		case COMMAND_ECHO_PORT:		// 1ポートエコーバック
			lret = DioEchoBackByte(id, port_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioEchoBackByte", lret, buf);
			}
			break;
		case COMMAND_ECHO_BIT:		// 1ビットエコーバック
			lret = DioEchoBackBit(id, bit_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioEchoBackBit", lret, buf);
			}
			break;
		case COMMAND_QUIT:			// 終了
			fprintf(stdout, "quit.\n");
			goto END;
			break;
		case COMMAND_ERROR:			// エラー
			fprintf(stderr, "error:%s", buf);
			break;
		}
	}
	//-------------------------------------
	// 終了
	//-------------------------------------
END:
	lret = DioExit(id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioExit", lret, buf);
		return -1;
	}
	return 0;
}


