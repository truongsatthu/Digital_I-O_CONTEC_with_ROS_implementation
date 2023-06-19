//================================================================
//================================================================
// CONTEC Linux BusMaster Sample for DIO
//			Terminal Operation Library Source File
//													CONTEC Co.,Ltd.
//													Ver1.00
//================================================================
//================================================================

#include "TermLib.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/ipc.h>
#include <linux/sem.h>


//=================================================
// グローバルデータ宣言
//=================================================
TERM_DATA	term_data;

//=================================================
// プロトタイプ宣言
//=================================================
static long TermSetInfoWindow(PSET_SCR_DATA set_scr);

//================================================================
// 初期化関数
//================================================================
long TermInit(PMAIN_SCR_DATA main_scr)
{
	char	buf[256];
	int		count, pos_y;
	int		lret;
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(main_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// グローバルデータ初期化
	//--------------------------------------
	memset(&term_data, 0, sizeof(TERM_DATA));
	//--------------------------------------
	// メニュー階層数取得
	//--------------------------------------
	for(count = 0; strcmp(main_scr->menu[count][0].name, "") && count < MENU_NUM; count++){}
	term_data.main_window.menu_tier	= count;
	//--------------------------------------
	// ステータスエリア名設定
	//--------------------------------------
	strcpy(term_data.status_top_name, main_scr->status_top_name);
	strcpy(term_data.status_buttom_name, main_scr->status_buttom_name);
	//--------------------------------------
	// 初期表示データ設定
	//--------------------------------------
	// ウインドウ名設定
	strcpy(term_data.caption_str, main_scr->app_name);
	// メニュー表示設定
	TermMenuSet(main_scr->menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 終了関数
//================================================================
long TermExit()
{
	//--------------------------------------
	// グローバルデータ初期化
	//--------------------------------------
	memset(&term_data, 0, sizeof(TERM_DATA));
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// メニュー設定関数
//================================================================
long TermMenuSet(PMENU_ITEM menu)
{
	int  count;
	char buf[256];
	
	//--------------------------------------
	// メニューデータコピー
	//--------------------------------------
	for(count = 0; count < MENU_NUM; count++){
		memmove(term_data.menu[count].name, menu[count].name, sizeof(char)*256);
	}
	//--------------------------------------
	// 仕切りの表示
	//--------------------------------------
	printf("\n==============================\n");
	puts(term_data.caption_str);
	printf("==============================\n");
	//--------------------------------------
	// メニュー表示設定
	//--------------------------------------
	for(count = 0; strcmp(menu[count].name, "") && count < MENU_NUM; count++){
		printf("%d.%s\n", count, menu[count].name);
	}
	//--------------------------------------
	// 仕切りの表示
	//--------------------------------------
	printf("==============================\n\n");

	return TERM_ERR_SUCCESS;
}

//================================================================
// メニュー番号取得関数
//================================================================
long TermGetMenuNum(char *message, int *menu_num)
{
	char get_char, buf[10];

	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(message == NULL || menu_num == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メッセージ表示
	//--------------------------------------
	printf("\n%s\n: ", message);
	//--------------------------------------
	// 数値入力(0-9)
	//--------------------------------------
	do{
		get_char = (char)getchar();
		buf[0]	= get_char;
		buf[1] 	= '\0';
		*menu_num = atoi(buf);
	}while((*menu_num == 0) && (get_char != '0'));
	
	//--------------------------------------
	// メニュー表示
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// 文字列取得関数
//================================================================
long TermGetStr(char *message, char *get_str)
{
	int y, x;
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(message == NULL || get_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メッセージ表示
	//--------------------------------------
	printf("%s\n: ", message);
	//--------------------------------------
	// 文字列入力
	//--------------------------------------
	scanf("%s", get_str);
	//--------------------------------------
	// メニュー表示
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// ステータス文字列設定関数
//================================================================
long TermSetStatus(int sts_area, char *item_name, char *status_str)
{
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(item_name == NULL || status_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// ステータス文字列設定
	//--------------------------------------
	if(sts_area == STATUS_AREA_TOP){
		printf("%s %-10s: %s\n", term_data.status_top_name, item_name, status_str);
	}else{
		printf("%s %-10s: %s\n", term_data.status_buttom_name, item_name, status_str);
	}

	return TERM_ERR_SUCCESS;
}

//================================================================
// 設定画面初期化関数
//================================================================
long TermSetWindowOpen(PSET_SCR_DATA set_scr)
{
	char buf[256];
	int count, pos_y, lret;
	
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メニュー階層数取得
	//--------------------------------------
	for(count = 0; strcmp(set_scr->menu[count][0].name, "") || count < MENU_NUM; count++){}
	term_data.set_window.menu_tier	= count;
	//--------------------------------------
	// 表示データ設定
	//--------------------------------------
	strcpy(term_data.pre_caption_str, term_data.caption_str);
	strcpy(term_data.caption_str, set_scr->window_name);
	//--------------------------------------
	// メニューデータコピー
	//--------------------------------------
	for(count = 0; count < MENU_NUM; count++){
		memmove(term_data.pre_menu[count].name, term_data.menu[count].name, sizeof(char)*256);
	}
	TermMenuSet(set_scr->menu[0]);
	//--------------------------------------
	// 設定画面情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 設定画面終了関数
//================================================================
long TermSetWindowClose()
{
	// ウインドウ名設定
	strcpy(term_data.caption_str, term_data.pre_caption_str);
	//--------------------------------------
	// メインメニューに戻る
	//--------------------------------------
	TermMenuSet(&term_data.pre_menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 設定画面情報表示関数
//================================================================
static long TermSetInfoWindow(PSET_SCR_DATA set_scr)
{
	SELECT_ITEM_DATA	*pitem_data;
	INPUT_NUM_DATA		*pnum_data;
	char buf[256];
	int count;
	
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// 仕切りの表示
	//--------------------------------------
	printf("\n------------------------------\n");
	//--------------------------------------
	// 項目選択タイプ設定情報表示
	//--------------------------------------
	for(count = 0; strcmp(set_scr->select_item[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT; count++){
		pitem_data	= &set_scr->select_item[count];
		printf("%-20s: %s\n", pitem_data->set_item_name, pitem_data->item[pitem_data->set_num].name);
	}
	//--------------------------------------
	// 数値入力タイプ設定情報表示
	//--------------------------------------
	for(count = 0; strcmp(set_scr->input_num[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT;  count++){
		pnum_data	= &set_scr->input_num[count];
		//--------------------------------------
		// 10進数表示タイプならば
		//--------------------------------------
		if(pnum_data->hex_or_dec == DEC_NUM){
			if(strcmp(pnum_data->unit_name, "")){
				printf("%-20s: %d [%s]\n", pnum_data->set_item_name, pnum_data->set_num, pnum_data->unit_name);
			}else{
				printf("%-20s: %d\n", pnum_data->set_item_name, pnum_data->set_num);
			}
		//--------------------------------------
		// 16進数表示タイプならば
		//--------------------------------------
		}else{
			if(strcmp(pnum_data->unit_name, "")){
				printf("%-20s: %X [%s]\n", pnum_data->set_item_name, pnum_data->set_num, pnum_data->unit_name);
			}else{
				printf("%-20s: %X\n", pnum_data->set_item_name, pnum_data->set_num);
			}
		}
	}
	//--------------------------------------
	// 仕切りの表示
	//--------------------------------------
	printf("------------------------------\n");

	return TERM_ERR_SUCCESS;
}	

//================================================================
// 設定内容取得関数(項目選択)
//================================================================
long TermGetSelectItem(PSET_SCR_DATA set_scr, int item_num, unsigned long *select_num)
{
	char buf[1000];
	int input_num, count, max_count, y, x;

	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// 項目名表示
	//--------------------------------------
	printf("< %s >\n", set_scr->select_item[item_num].set_item_name);
	//--------------------------------------
	// 質問表示
	//--------------------------------------
	printf("Please choose from the following choice.\n");
	//--------------------------------------
	// 選択項目表示
	//--------------------------------------
	strcpy(buf, "\n");
	for(count = 0; strcmp(set_scr->select_item[item_num].item[count].name, "") && count < SELECT_ITEM_MAX_COUNT; count++){
		sprintf(buf, "%s%d. %s  ", buf, count + 1, set_scr->select_item[item_num].item[count].name);
	}
	// 最大項目数保存
	max_count = count;
	for(;;){
		//--------------------------------------
		// 回答入力
		//--------------------------------------
		printf("%s\n> ", buf);
		scanf("%s", buf);
		input_num = atoi(buf);
		if(0 < input_num && input_num <=  max_count){
			break;
		}
	}
	//--------------------------------------
	// 入力値を設定画面データに設定
	//--------------------------------------
	set_scr->select_item[item_num].set_num = input_num - 1;
	//--------------------------------------
	// 返却データを設定
	//--------------------------------------
	*select_num	= set_scr->select_item[item_num].item[input_num - 1].num;
	//--------------------------------------
	// 設定情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// メニュー表示
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 設定内容取得関数(数値入力)
//================================================================
long TermGetInputNum(PSET_SCR_DATA set_scr, int item_num, unsigned long *input_num)
{
	char buf[1000];
	int count, y, x;

	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// 項目名表示
	//--------------------------------------
	printf("< %s >\n", set_scr->input_num[item_num].set_item_name);
	//--------------------------------------
	// 質問表示
	//--------------------------------------
	printf("Please input a setting value.\n");
	// プロンプト表示
	if(strcmp(set_scr->input_num[item_num].unit_name, "")){
		printf("[%s]:", set_scr->input_num[item_num].unit_name);
	}else{
		printf(":");
	}
	//--------------------------------------
	// 回答入力
	//--------------------------------------
	// 10進数ならば
	if(set_scr->input_num[item_num].hex_or_dec == DEC_NUM){
		scanf("%d", input_num);
	// 16進数ならば
	}else{
		scanf("%x", input_num);
	}
	//--------------------------------------
	// 入力値を設定画面データに設定
	//--------------------------------------
	set_scr->input_num[item_num].set_num = *input_num;
	//--------------------------------------
	// 設定情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// メニュー表示
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);
	
	return TERM_ERR_SUCCESS;
}

