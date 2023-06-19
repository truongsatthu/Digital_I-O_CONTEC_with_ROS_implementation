//================================================================
//================================================================
//	CONTEC Linux DIO (BusMaster)
//	Input Sample
// 										CONTEC Co.,Ltd.
//										Ver1.00
//================================================================
//================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../inc/cdio.h"
#include "../common/TermLib.h"


//================================================================
// マクロ定義
//================================================================
#define	MENU_DIO_OPEN		0
#define	MENU_CONDITION		1
#define	MENU_START			2
#define	MENU_STOP			3
#define	MENU_DATA			4
#define	MENU_COUNT			5
#define	NENU_STATUS			6
#define	MENU_DIO_CLOSE		7
#define	MENU_EXIT			8
#define	SET_MENU_START		0
#define	SET_MENU_CLOCK		1
#define	SET_MENU_STOP		2
#define	SET_MENU_NOTIF		3
#define	SET_MENU_OK			4
#define	SET_MENU_CANCEL		5
#define	STS_COUNT_POS		1
#define	STS_STATUS_POS		2
#define	STS_ERROR_POS		3
#define	STS_RET_POS			4
#define	SET_NUM_COMP_PTN	0
#define	SET_NUM_PTN_MASK	1
#define	SET_NUM_CLOCK		2
#define	SET_NUM_STOP_NUM	3
#define	SET_NUM_NOTIF		4

#define	DATA_SIZE			1000					// データサイズ
//================================================================
// 外部変数
//================================================================
static char				file_name[] = "input.txt";	// 保存ファイル名
static unsigned long	data_buff[DATA_SIZE];		// データバッファ

//-----------------------------------
//	メイン画面データ
//-----------------------------------
MAIN_SCR_DATA	main_scr = 
{
	"<< Input Sample >>",
	"",
	"",
	{
		{{"DioInit"},{"Condition..."},{"Start"},
		{"Stop"},{"Data"},{"Count"},{"Status"},{"DioExit"},{"Exit"},
		{""}},
		{{""}}
	},
	{{""}},
	{{"Count"},{"Status"},{"Error"},{"Ret"},{""}}
};

//-----------------------------------
//	設定画面データ
//-----------------------------------
SET_SCR_DATA	set_scr = 
{
	"<< Sampling Condition >>",
	{
		{{"Start"},{"Clock"},{"Stop"},
		{"Notification"},{"OK"},{"Cancel"},{""},
		{""},{""},{""}},
		{{""}}
	},
	{	//	項目選択タイプ用設定データ
		{
			0,
			"Start",
			{
				{"Software Start",	DIODM_START_SOFT},
				{"External UP",		DIODM_START_EXT_RISE},
				{"External DOWN",	DIODM_START_EXT_FALL},
				{"Pattern",			DIODM_START_PATTERN},
				{""}
			},
			0
		},
		{
			2,
			"Clock",
			{
				{"Internal Clock",	DIODM_CLK_CLOCK},
				{"External Clock",	DIODM_CLK_EXT_TRG},
				{"Hand Shake",		DIODM_CLK_HANDSHAKE},
				{""}
			},
			0
		},
		{
			3,
			"Stop",
			{
				{"Software Stop", 	DIODM_STOP_SOFT},
				{"External UP",		DIODM_STOP_EXT_RISE},
				{"External DOWN",	DIODM_STOP_EXT_FALL},
				{"Number",			DIODM_STOP_NUM},
				{""}
			},
			0
		}
	},
	{	//	数値入力タイプ用設定データ
		{0, "Compare Pattern",	"Hex",	0x55,	HEX_NUM},
		{1, "Pattern Mask",		"Hex",	0xFF,	HEX_NUM},
		{2, "Internal Clock",	"ns",	1000,	DEC_NUM},
		{3, "Stop Number",		"",		1000,	DEC_NUM},
		{4, "Number of Data",	"",		500,	DEC_NUM}
	}
};

//================================================================
//	転送完了コールバック関数
//================================================================
void stop_call_back(short id, short message, unsigned long dir, void *param)
{
	char buf[256];
	//------------------------------------------
	// 割り込みメッセージの処理
	//------------------------------------------
	if (message == DIO_DMM_STOP){
		TermSetStatus(STATUS_AREA_BOTTOM, STS_STATUS_POS, "Stopped!");
	//------------------------------------------
	// それ以外
	//------------------------------------------
	}else{
        sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
    }
    return;
}

//================================================================
//	指定個数転送完了コールバック関数
//================================================================
void count_call_back(short id, short message, unsigned long dir, void *param)
{
	char get_str[256], buf[256];
	unsigned long count, carry;
	long lret;
	//------------------------------------------
	// 割り込みメッセージの処理
	//------------------------------------------
	if (message == DIO_DMM_COUNT){
		lret = DioDmGetCount(id, dir, &count, &carry);
		if(lret == TERM_ERR_SUCCESS){
			sprintf(buf, "%lu", count);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, buf);
		}
		DioGetErrorString(lret, get_str);
		sprintf(buf, "DioDmGetCount %ld:%s", lret, get_str);
		TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
	//------------------------------------------
	// それ以外
	//------------------------------------------
	}else{
        sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, buf);
    }
    return;
}

//================================================================
//	転送条件設定関数
//================================================================
long condition(short id, char *err_str)
{
	char			get_str[256];
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN; 
	unsigned long	start, clock, stop;
	unsigned long	comp_ptn, ptn_mask, internal_clock, stop_num, notification;
	unsigned long	def_set_num;
	long 			lret;
	//-----------------------------------
	// 設定データ初期化
	//-----------------------------------
	def_set_num		= set_scr.select_item[SET_MENU_START].set_num;
	start			= set_scr.select_item[SET_MENU_START].item[def_set_num].num;
	def_set_num		= set_scr.select_item[SET_MENU_CLOCK].set_num;
	clock			= set_scr.select_item[SET_MENU_CLOCK].item[def_set_num].num;
	def_set_num		= set_scr.select_item[SET_MENU_STOP].set_num;
	stop			= set_scr.select_item[SET_MENU_STOP].item[def_set_num].num;
	comp_ptn		= set_scr.input_num[SET_NUM_COMP_PTN].set_num;
	ptn_mask		= set_scr.input_num[SET_NUM_PTN_MASK].set_num;
	internal_clock	= set_scr.input_num[SET_NUM_CLOCK].set_num;
	stop_num		= set_scr.input_num[SET_NUM_STOP_NUM].set_num;
	notification	= set_scr.input_num[SET_NUM_NOTIF].set_num;
	//-----------------------------------
	// 設定ウインドウ初期化
	//-----------------------------------
	lret = TermSetWindowOpen(&set_scr);
	if(lret != TERM_ERR_SUCCESS){
		sprintf(err_str, "TermSetWindowOpen %ld\n", lret);
		return 1;
	}
	//-----------------------------------
	// メニュー処理
	//-----------------------------------
	strcpy(err_str, "");
	while(1){
		//-----------------------
		// 前回エラーだったら設定画面終了
		//-----------------------
		if(strcmp(err_str, "")){
			TermSetWindowClose();
			return 1;
		}
		//-----------------------
		// メニュー番号入力
		//-----------------------
		lret = TermGetMenuNum("Please input a menu number(0-5).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			sprintf(err_str, "TermGetMenuNum %ld\n", lret);
			return 1;
		}
		switch(menu_num){
		//-----------------------------------
		// スタート条件設定
		//-----------------------------------
		case SET_MENU_START:
			lret = TermGetSelectItem(&set_scr, SET_MENU_START, &start);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(start == DIODM_START_PATTERN){
				lret = TermGetInputNum(&set_scr, SET_NUM_COMP_PTN, &comp_ptn);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
				lret = TermGetInputNum(&set_scr, SET_NUM_PTN_MASK, &ptn_mask);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// クロック条件設定
		//-----------------------------------
		case SET_MENU_CLOCK:
			lret = TermGetSelectItem(&set_scr, SET_MENU_CLOCK, &clock);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(clock == DIODM_CLK_CLOCK){
				lret = TermGetInputNum(&set_scr, SET_NUM_CLOCK, &internal_clock);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// ストップ条件設定
		//-----------------------------------
		case SET_MENU_STOP:
			lret = TermGetSelectItem(&set_scr, SET_MENU_STOP, &stop);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(stop == DIODM_STOP_NUM){
				lret = TermGetInputNum(&set_scr, SET_NUM_STOP_NUM, &stop_num);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// 通知カウント設定
		//-----------------------------------
		case SET_MENU_NOTIF:
			lret = TermGetInputNum(&set_scr, SET_NUM_NOTIF, &notification);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetInputNum %ld\n", lret);
			}
			break;
		//-----------------------------------
		// OK処理
		//-----------------------------------
		case SET_MENU_OK:
			// Direction
			lret = DioDmSetDirection(id, PI_32);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetDirection %ld:%s", lret, get_str);
				break;
			}
			// StandAlone
			lret = DioDmSetStandAlone(id);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStandAlone %ld:%s", lret, get_str);
				break;
			}
			// Start Condition
			lret = DioDmSetStartTrg(id, dir, start);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStartTrg %ld:%s", lret, get_str);
				break;
			}
			// Clock Condition
			lret = DioDmSetClockTrg(id, dir, clock);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetClockTrg %ld:%s", lret, get_str);
				break;
			}
			// Stop Condition
			lret = DioDmSetStopTrg(id, dir, stop);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStopTrg %ld:%s", lret, get_str);
				break;
			}

			// Pattern/Mask
			lret = DioDmSetStartPattern(id, comp_ptn, ptn_mask);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStartPattern %ld:%s", lret, get_str);
				break;
			}
			// Internal Clock
			lret = DioDmSetInternalClock(id, dir, internal_clock, DIODM_TIM_UNIT_NS);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetInternalClock %ld:%s", lret, get_str);
				break;
			}
			// StopNumber
			lret = DioDmSetStopNum(id, dir, stop_num);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStopNum %ld:%s", lret, get_str);
				break;
			}
		//-----------------------------------
		// Cancel処理
		//-----------------------------------
		case SET_MENU_CANCEL:
			//-----------------------------------
			// 設定画面終了
			//-----------------------------------
			TermSetWindowClose();
			return 0;
			break;
		}
	}

	return 0;
}

//================================================================
//	メイン関数
//================================================================
int main()
{
	short			id;
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN; 
	unsigned long	i, count, carry, status, err, notification;
	char			get_str[256], buf[256];
	FILE			*fp;
	long 			lret;
	
	//-----------------------------------
	// TermLib初期化
	//-----------------------------------
	lret = TermInit(&main_scr);
	if(lret != TERM_ERR_SUCCESS){
		printf("TermInit RetCode = %ld\n", lret);
		return 0;
	}
	//-----------------------------------
	// メニュー処理
	//-----------------------------------
	while(1){
		lret = TermGetMenuNum("Please input a menu number(0-8)", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			printf("TermGetMenuNum RetCode = %ld\n", lret);
			return 0;
		}
		switch(menu_num){
		//--------------------------------------------
		// 初期化処理
		//--------------------------------------------
		case MENU_DIO_OPEN:
			lret = TermGetStr("Please input a device name.", get_str);
			if(lret != TERM_ERR_SUCCESS){
				printf("TermGetStr RetCode = %ld\n", lret);
				return 0;
			}
			lret = DioInit(get_str, &id);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioInit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// 設定
		//--------------------------------------------
		case MENU_CONDITION:
			lret = condition(id, get_str);
			if( lret == 1){
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, get_str);
			}
			break;
		//--------------------------------------------
		// 転送スタート
		//--------------------------------------------
		case MENU_START:
			//----------------------------
			// Status clear
			//----------------------------
			TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, "");
			TermSetStatus(STATUS_AREA_BOTTOM, STS_STATUS_POS, "");
			TermSetStatus(STATUS_AREA_BOTTOM, STS_ERROR_POS, "");
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, "");
			//----------------------------
			// Notification
			//----------------------------
			lret = DioDmSetStopEvent(id, dir,(PDIO_STOP_CALLBACK)stop_call_back, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetStopEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			notification = set_scr.input_num[SET_NUM_NOTIF].set_num;
			lret = DioDmSetCountEvent(id, dir, notification, (PDIO_COUNT_CALLBACK)count_call_back, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetCountEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			    break;
			}
			//----------------------------
			// Reset
			//----------------------------
			lret = DioDmReset(id, DIODM_RESET_FIFO_IN);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmReset %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			    break;
			}
			//----------------------------
			// Set Buffer
			//----------------------------
			lret = DioDmSetBuff(id, dir, data_buff, DATA_SIZE, DIODM_WRITE_ONCE);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetBuff %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			    break;
			}
			//----------------------------
			// Start
			//----------------------------
			lret = DioDmStart(id, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStart %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// 転送停止
		//--------------------------------------------
		case MENU_STOP:
			lret = DioDmStop(id, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStop %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// データをファイルに保存
		//--------------------------------------------
		case MENU_DATA:
			//----------------------------
			// ファイルオープン
			//----------------------------
			if ((fp = fopen(file_name, "w")) == NULL) {
				strcpy(buf, "fopen : file open error.");
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// ファイルに書き込み
			//----------------------------
			for (i=0; i<DATA_SIZE; i++) {
				sprintf(buf, "%8ld : %08lX\n", i, data_buff[i]);
				fputs(buf, fp); 
			}
			//----------------------------
			// ファイルクローズ
			//----------------------------
			fclose(fp);
			sprintf(buf, "%s にデータを保存しました。", file_name);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// 転送カウント取得
		//--------------------------------------------
		case MENU_COUNT:
			lret = DioDmGetCount(id, dir, &count, &carry);
			if(lret == TERM_ERR_SUCCESS){
				sprintf(buf, "%lu", count);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, buf);
			}
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmGetCount %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// ステータス取得
		//--------------------------------------------
		case NENU_STATUS:
			lret = DioDmGetStatus(id, dir, &status, &err);
			if(lret == TERM_ERR_SUCCESS){
				// Status
				strcpy(buf, "");
				if (status & DIODM_STATUS_BMSTOP)	strcpy(buf, "BmStop,");
				if (status & DIODM_STATUS_PIOSTART)	strcat(buf, "PioStart,");
				if (status & DIODM_STATUS_PIOSTOP)	strcat(buf, "PioStop,");
				if (status & DIODM_STATUS_TRGIN)	strcat(buf, "TrgIn,");
				if (status & DIODM_STATUS_OVERRUN)	strcat(buf, "Overrun");
				TermSetStatus(STATUS_AREA_BOTTOM, STS_STATUS_POS, buf);
				// Error
				strcpy(buf, "");
				if (err & DIODM_STATUS_FIFOEMPTY)	strcpy(buf, "FifoEmpty,");
				if (err & DIODM_STATUS_FIFOFULL)	strcat(buf, "FifoFull,");
				if (err & DIODM_STATUS_SGOVERIN)	strcat(buf, "S/GOverIn,");
				if (err & DIODM_STATUS_TRGERR)		strcat(buf, "TrgErr,");
				if (err & DIODM_STATUS_CLKERR)		strcat(buf, "ClkErr,");
				if (err & DIODM_STATUS_SLAVEHALT)	strcat(buf, "SlaveHalt,");
				if (err & DIODM_STATUS_MASTERHALT)	strcat(buf, "MasterHalt");
				TermSetStatus(STATUS_AREA_BOTTOM, STS_ERROR_POS, buf);
			}
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmGetStatus %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// 終了処理
		//--------------------------------------------
		case MENU_DIO_CLOSE:
			lret = DioExit(id);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioExit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// サンプル終了
		//--------------------------------------------
		case MENU_EXIT:	
			TermExit();
			return 0;
			break;
		}
	}
	return 0;
}


