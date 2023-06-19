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
#include <curses.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/ipc.h>
#include <linux/sem.h>

//=================================================
// マクロ定義
//=================================================
#define	SCR_LENGTH				80
#define	SCR_HIGHT				25
#define	SCR_LINE_WIDTH			1
#define	MENU_AREA_LENGTH		15
#define	SCR_CAP_HIGHT			1
#define	MAIN_SCR_STS_HIGHT		7
#define	SET_SCR_INFO_HIGHT		10
#define	MAIN_SCR_MSG_HIGHT		SCR_HIGHT - SCR_CAP_HIGHT - MAIN_SCR_STS_HIGHT*2 - SCR_LINE_WIDTH*5
#define	SET_SCR_MSG_HIGHT		SCR_HIGHT - SCR_CAP_HIGHT - SET_SCR_INFO_HIGHT - SCR_LINE_WIDTH*4
#define	SCR_MSG_LENGTH			SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3
#define	MSG_AREA_START_POS_X	MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2
#define	MAIN_STS_OFFSET			10
#define	INFO_STR_NAME_OFFSET	1
#define	INFO_STR_OFFSET			16
#define	INFO_NUM_NAME_OFFSET	32
#define	INFO_NUM_OFFSET			51


//=================================================
// グローバルデータ宣言
//=================================================
TERM_DATA	term_data;

//=================================================
// プロトタイプ宣言
//=================================================
static long TermWindowCreate(int area);
static long TermSetInfoWindow(PSET_SCR_DATA set_scr);

//=================================================
// 端末サイズ変更シグナル処理
//=================================================
void TermResize(int sig)
{
	//--------------------------------------
	// curses側シグナル処理呼び出し
	//--------------------------------------
	if (term_data.org_resize != NULL) {
		term_data.org_resize(sig);
	}
	//--------------------------------------
	// 表示更新
	//--------------------------------------
	if (term_data.active_window->pmain_window != NULL){
		prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES - 1, COLS - 1);
	}
}

//================================================================
// セマフォ設定関数
//================================================================
static int SetSemaphore(void)
{
	union semun sem_union;

	term_data.sem_id	= semget((key_t)1000, 1, 0666 | IPC_CREAT);
	sem_union.val		= 1;
	if (semctl(term_data.sem_id, 0, SETVAL, sem_union) == -1){
		return TERM_ERR_INIT;
	}
	return TERM_ERR_SUCCESS;
}

//================================================================
// セマフォ削除関数
//================================================================
static void DelSemaphore(void)
{
	union semun sem_union;

	semctl(term_data.sem_id, 0, IPC_RMID, sem_union);
}

//================================================================
// セマフォ p操作
//================================================================
static void SemaphoreP(void)
{
	struct sembuf sem_b;

	sem_b.sem_num	= 0;
	sem_b.sem_op	= -1;
	sem_b.sem_flg	= SEM_UNDO;
	semop(term_data.sem_id, &sem_b, 1);
}

//================================================================
// セマフォ v操作
//================================================================
static void SemaphoreV(void)
{
	struct sembuf sem_b;

	sem_b.sem_num	= 0;
	sem_b.sem_op	= 1;
	sem_b.sem_flg	= SEM_UNDO;
	semop(term_data.sem_id, &sem_b, 1);
}

//================================================================
// ウインドウ生成関数
//================================================================
static long TermWindowCreate(int area)
{
	PWINDOW_DATA	pwnd;

	//--------------------------------------
	// 生成するウインドウの構造体を選択
	//--------------------------------------
	if(area == MAIN_AREA){
		pwnd	= &term_data.main_window;
	}else{
		pwnd	= &term_data.set_window;
	}
	//--------------------------------------
	// メイン画面生成
	//--------------------------------------
	pwnd->pmain_window = newpad(SCR_HIGHT, SCR_LENGTH);
	// エラーチェック
	if(pwnd->pmain_window == NULL){
		return TERM_ERR_INIT_SCR;
	}
	//--------------------------------------
	// サブ画面生成
	//--------------------------------------
	// キャプションエリア生成
	pwnd->pcap_window			= subpad(pwnd->pmain_window, SCR_CAP_HIGHT, SCR_LENGTH, 1, 0);
	// メニューエリア生成
	pwnd->pmenu_window			= subpad(pwnd->pmain_window, 
										SCR_HIGHT - SCR_CAP_HIGHT - SCR_LINE_WIDTH * 3, MENU_AREA_LENGTH, 
										SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, SCR_LINE_WIDTH);
	// エラーチェック
	if(pwnd->pcap_window == NULL || pwnd->pmenu_window == NULL){
		return TERM_ERR_INIT_SCR;
	}
	//--------------------------------------
	// メインウインドウならば
	//--------------------------------------
	if(area == MAIN_AREA){
		// メッセージエリア生成
		pwnd->pmessage_window		= subpad(pwnd->pmain_window, MAIN_SCR_MSG_HIGHT, SCR_MSG_LENGTH,
											SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// 上部ステータスエリア生成
		pwnd->psts_top_window		= subpad(pwnd->pmain_window, MAIN_SCR_STS_HIGHT,  SCR_MSG_LENGTH,
											SCR_HIGHT - MAIN_SCR_STS_HIGHT * 2 - SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// 下部ステータスエリア生成
		pwnd->psts_bottom_window	= subpad(pwnd->pmain_window, MAIN_SCR_STS_HIGHT,  SCR_MSG_LENGTH,
											SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH, MSG_AREA_START_POS_X);
		// エラーチェック
		if(	pwnd->pmessage_window == NULL || pwnd->psts_top_window == NULL || pwnd->psts_bottom_window == NULL){
		return TERM_ERR_INIT_SCR;
		}
	//--------------------------------------
	// 設定ウインドウならば
	//--------------------------------------
	}else{
		// メッセージエリア生成
		pwnd->pmessage_window		= subpad(pwnd->pmain_window, MAIN_SCR_MSG_HIGHT, SCR_MSG_LENGTH,
											SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// 設定情報エリア生成
		pwnd->pset_info_window		= subpad(pwnd->pmain_window, SET_SCR_INFO_HIGHT, SCR_MSG_LENGTH,
											SCR_HIGHT - SET_SCR_INFO_HIGHT - SCR_LINE_WIDTH, MSG_AREA_START_POS_X);
		// エラーチェック
		if(	pwnd->pmessage_window == NULL || pwnd->pset_info_window == NULL){
		return TERM_ERR_INIT_SCR;
		}
	}
	//--------------------------------------
	// ライン生成
	//--------------------------------------
	//--------------------------------------
	// 全体囲み
	//--------------------------------------
	box(pwnd->pmain_window, ACS_VLINE, ACS_HLINE);
	//--------------------------------------
	// 横ライン
	//--------------------------------------
	mvwhline(pwnd->pmain_window, SCR_CAP_HIGHT + SCR_LINE_WIDTH, SCR_LINE_WIDTH, ACS_HLINE, SCR_LENGTH - SCR_LINE_WIDTH * 2);
	// メインウインドウならば
	if(area == MAIN_AREA){
		mvwhline(pwnd->pmain_window, 
				SCR_CAP_HIGHT + MAIN_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
		mvwhline(pwnd->pmain_window, 
				SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
	// 設定ウインドウならば
	}else{
		mvwhline(pwnd->pmain_window, 
				SCR_CAP_HIGHT + SET_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
	}
	//--------------------------------------
	// 縦ライン
	//--------------------------------------
	mvwvline(pwnd->pmain_window, 
			SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH, 
			ACS_VLINE, SCR_HIGHT - SCR_CAP_HIGHT - SCR_LINE_WIDTH * 3);

	return TERM_ERR_SUCCESS;
}

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
	// セマフォ設定
	//--------------------------------------
	lret = SetSemaphore();
	if(lret != TERM_ERR_SUCCESS){
		return lret;
	}
	//--------------------------------------
	// curses 初期化
	//--------------------------------------
	term_data.std_window = initscr();
	if(term_data.std_window == NULL){
		return TERM_ERR_INIT;
	}
	//--------------------------------------
	// curses モード設定
	//--------------------------------------
	cbreak();
	curs_set(0);
	//--------------------------------------
	// メインウインドウ生成
	//--------------------------------------
	lret = TermWindowCreate(MAIN_AREA);
	if(lret != TERM_ERR_SUCCESS){
		//--------------------------------------
		// curses終了処理
		//--------------------------------------
		endwin();
		return lret;
	}
	//--------------------------------------
	// アクティブウインドウ登録
	//--------------------------------------
 	term_data.active_window	= &term_data.main_window;
	//--------------------------------------
	// ウインドウサイズ変更シグナル登録
	//--------------------------------------
	term_data.org_resize	= signal(SIGWINCH, TermResize);
	//--------------------------------------
	// 初期表示データ設定
	//--------------------------------------
	// ウインドウ名設定
	mvwaddstr(term_data.main_window.pcap_window, 0, 3, main_scr->app_name);
	// 上部ステータスエリア名設定
	mvwaddstr(term_data.main_window.pmain_window,
				SCR_CAP_HIGHT + MAIN_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2 + 1, 
				main_scr->status_top_name);
	// 下部ステータスエリア名設定
	mvwaddstr(term_data.main_window.pmain_window, 
				SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2 + 1, 
				main_scr->status_buttom_name);
	// メニュー表示設定
	TermMenuSet(main_scr->menu[0]);
	// 上部ステータス表示設定
	for(count = 0, pos_y = 1; strcmp(main_scr->status_top[count].label, "") && count < STATUS_NUM; count++, pos_y++){
		sprintf(buf, "%s:", main_scr->status_top[count].label);
		mvwaddstr(term_data.main_window.psts_top_window, pos_y, 1, buf);
	}
	// 下部ステータス表示設定
	for(count = 0, pos_y = 1; strcmp(main_scr->status_buttom[count].label, "") && count < STATUS_NUM; count++, pos_y++){
		sprintf(buf, "%s:", main_scr->status_buttom[count].label);
		mvwaddstr(term_data.main_window.psts_bottom_window, pos_y, 1, buf);
	}
	wmove(term_data.main_window.pmain_window, 0, 0);
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 終了関数
//================================================================
long TermExit()
{
	//--------------------------------------
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// ウインドウ削除
	//--------------------------------------
	delwin(term_data.main_window.psts_bottom_window);
	delwin(term_data.main_window.psts_top_window);
	delwin(term_data.main_window.pmessage_window);
	delwin(term_data.main_window.pmenu_window);
	delwin(term_data.main_window.pcap_window);
	delwin(term_data.main_window.pmain_window);
	//--------------------------------------
	// curses モード設定
	//--------------------------------------
	nocbreak();
	//--------------------------------------
	// curses終了処理
	//--------------------------------------
	endwin();
	//--------------------------------------
	// セマフォ削除
	//--------------------------------------
	DelSemaphore();
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
	int  count, pos_y;
	char buf[256];
	
	//--------------------------------------
	// メニューエリアクリア
	//--------------------------------------
	wclear(term_data.active_window->pmenu_window);
	//--------------------------------------
	// メニュー表示設定
	//--------------------------------------
	for(count = 0, pos_y = 1; strcmp(menu[count].name, "") && count < MENU_NUM; count++, pos_y ++){
		sprintf(buf, "%d.%s", count, menu[count].name);
		mvwaddstr(term_data.active_window->pmenu_window, pos_y, 1, buf);
	}
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// メニュー番号取得関数
//================================================================
long TermGetMenuNum(char *message, int *menu_num)
{
	char get_char, buf[10];
	//--------------------------------------
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(message == NULL || menu_num == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メッセージエリアクリア
	//--------------------------------------
	wclear(term_data.active_window->pmessage_window);
	//--------------------------------------
	// メッセージ表示
	//--------------------------------------
	mvwaddstr(term_data.active_window->pmessage_window, 0, 1, message);
	//--------------------------------------
	// 入力バッファを破棄
	//--------------------------------------
	flushinp();
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// 数値入力(0-9)
	//--------------------------------------
	do{
		get_char = wgetch(term_data.active_window->pmessage_window);
		buf[0]	= get_char;
		buf[1] 	= '\0';
		*menu_num = atoi(buf);
	}while((*menu_num == 0) && (get_char != '0'));
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// 文字列取得関数
//================================================================
long TermGetStr(char *message, char *get_str)
{
	int y, x;
	//--------------------------------------
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(message == NULL || get_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メッセージエリアクリア
	//--------------------------------------
	wclear(term_data.active_window->pmessage_window);
	//--------------------------------------
	// メッセージ表示
	//--------------------------------------
	mvwaddstr(term_data.active_window->pmessage_window, 0, 1, message);
	waddstr(term_data.active_window->pmessage_window, "\n :");
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// 現在の画面を標準スクリーンにコピー
	// パッドでは入力文字が表示されないため
	//--------------------------------------
	overwrite(term_data.active_window->pmain_window, term_data.std_window);
	getyx(term_data.active_window->pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	move(y, x);
	//--------------------------------------
	// 文字列入力
	//--------------------------------------
	curs_set(1);
	getstr(get_str);
	curs_set(0);
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// ステータス文字列設定関数
//================================================================
long TermSetStatus(int sts_area, int pos_y, char *status_str)
{
	//--------------------------------------
	// セマフォP操作
	//--------------------------------------
	SemaphoreP();
	//--------------------------------------
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(status_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// ステータス文字列設定
	//--------------------------------------
	if(sts_area == STATUS_AREA_TOP){
		mvwaddstr(term_data.main_window.psts_top_window, pos_y, MAIN_STS_OFFSET, status_str);
		wclrtoeol(term_data.main_window.psts_top_window);
	}else{
		mvwaddstr(term_data.main_window.psts_bottom_window, pos_y, MAIN_STS_OFFSET, status_str);
		wclrtoeol(term_data.main_window.psts_bottom_window);
	}
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// セマフォV操作
	//--------------------------------------
	SemaphoreV();

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
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// 重複実行エラー
	//--------------------------------------
	if(term_data.set_window.pmain_window != NULL){
		return TERM_ERR_DUP_EXEC;
	}
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
	// 設定画面生成
	//--------------------------------------
	lret = TermWindowCreate(SET_AREA);
	if(lret != TERM_ERR_SUCCESS){
		return lret;
	}
	//--------------------------------------
	// アクティブウインドウ登録
	//--------------------------------------
	term_data.active_window	= &term_data.set_window;
	//--------------------------------------
	// 初期表示データ設定
	//--------------------------------------
	// ウインドウ名設定
	mvwaddstr(term_data.set_window.pcap_window, 0, 3, set_scr->window_name);
	// メニュー表示設定
	TermMenuSet(set_scr->menu[0]);
	//--------------------------------------
	// 設定画面情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// カーソルを左上に移動
	//--------------------------------------
	wmove(term_data.active_window->pmain_window, 0, 0);
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// 設定画面終了関数
//================================================================
long TermSetWindowClose()
{
	//--------------------------------------
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// アクティブウインドウ登録
	//--------------------------------------
 	term_data.active_window	= &term_data.main_window;
	//--------------------------------------
	// ウインドウ削除
	//--------------------------------------
	delwin(term_data.set_window.pset_info_window);
	delwin(term_data.set_window.pmessage_window);
	delwin(term_data.set_window.pmenu_window);
	delwin(term_data.set_window.pcap_window);
	delwin(term_data.set_window.pmain_window);
	//--------------------------------------
	// 設定画面ウインドウ構造体初期化
	//--------------------------------------
	memset(&term_data.set_window, 0, sizeof(WINDOW_DATA));
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

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
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// メッセージエリアクリア
	//--------------------------------------
	wclear(term_data.set_window.pset_info_window);
	//--------------------------------------
	// 項目選択タイプ設定情報表示
	//--------------------------------------
	for(count = 0; strcmp(set_scr->select_item[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT; count++){
		pitem_data	= &set_scr->select_item[count];
		sprintf(buf, "%s:", pitem_data->set_item_name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_STR_NAME_OFFSET, buf);
		strcpy(buf, pitem_data->item[pitem_data->set_num].name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_STR_OFFSET, buf);
	}
	//--------------------------------------
	// 数値入力タイプ設定情報表示
	//--------------------------------------
	for(count = 0; strcmp(set_scr->input_num[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT;  count++){
		pnum_data	= &set_scr->input_num[count];
		sprintf(buf, "%s:", pnum_data->set_item_name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_NUM_NAME_OFFSET, buf);
		//--------------------------------------
		// 10進数表示タイプならば
		//--------------------------------------
		if(pnum_data->hex_or_dec == DEC_NUM){
			if(strcmp(pnum_data->unit_name, "")){
				sprintf(buf, "%d [%s]", pnum_data->set_num, pnum_data->unit_name);
			}else{
				sprintf(buf, "%d", pnum_data->set_num);
			}
		//--------------------------------------
		// 16進数表示タイプならば
		//--------------------------------------
		}else{
			if(strcmp(pnum_data->unit_name, "")){
				sprintf(buf, "%X [%s]", pnum_data->set_num, pnum_data->unit_name);
			}else{
				sprintf(buf, "%X", pnum_data->set_num);
			}
		}
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_NUM_OFFSET, buf);
	}
	//--------------------------------------
	// 表示データを画面に反映
	//--------------------------------------
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

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
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// 項目名表示
	//--------------------------------------
	sprintf(buf, "< %s >\n", set_scr->select_item[item_num].set_item_name);
	mvwaddstr(term_data.set_window.pmessage_window, 0, 0, buf);
	//--------------------------------------
	// 質問表示
	//--------------------------------------
	waddstr(term_data.set_window.pmessage_window, "Please choose from the following choice.\n");
	//--------------------------------------
	// 選択項目表示
	//--------------------------------------
	strcpy(buf, "\n");
	for(count = 0; strcmp(set_scr->select_item[item_num].item[count].name, "") && count < SELECT_ITEM_MAX_COUNT; count++){
		sprintf(buf, "%s%d. %s  ", buf, count + 1, set_scr->select_item[item_num].item[count].name);
	}
	// 最大項目数保存
	max_count = count;
	waddstr(term_data.set_window.pmessage_window, buf);
	// プロンプト表示
	waddstr(term_data.set_window.pmessage_window, " > ");
	// 表示データを画面に反映
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// 現在の画面を標準スクリーンにコピー
	// パッドでは入力文字が表示されないため
	//--------------------------------------
	overwrite(term_data.set_window.pmain_window, term_data.std_window);
	getyx(term_data.set_window.pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	for(;;){
		//--------------------------------------
		// 回答入力
		//--------------------------------------
		move(y, x);
		refresh();
		curs_set(1);
		getstr(buf);
		curs_set(0);
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
	// メッセージエリアクリア
	//--------------------------------------
	wclear(term_data.set_window.pmessage_window);
	//--------------------------------------
	// 設定情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);

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
	// 初期化チェック
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// パラメータチェック
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// 項目名表示
	//--------------------------------------
	sprintf(buf, "< %s >\n", set_scr->input_num[item_num].set_item_name);
	mvwaddstr(term_data.set_window.pmessage_window, 0, 0, buf);
	//--------------------------------------
	// 質問表示
	//--------------------------------------
	waddstr(term_data.set_window.pmessage_window, "Please input a setting value.\n");
	if(strcmp(set_scr->input_num[item_num].unit_name, "")){
		sprintf(buf, "[%s]:", set_scr->input_num[item_num].unit_name);
	}else{
		sprintf(buf, ":");
	}
	// プロンプト表示
	waddstr(term_data.set_window.pmessage_window, buf);
	// 表示データを画面に反映
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// 現在の画面を標準スクリーンにコピー
	// パッドでは入力文字が表示されないため
	//--------------------------------------
	overwrite(term_data.set_window.pmain_window, term_data.std_window);
	getyx(term_data.set_window.pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	move(y, x);
	refresh();
	curs_set(1);
	//--------------------------------------
	// 回答入力
	//--------------------------------------
	// 10進数ならば
	if(set_scr->input_num[item_num].hex_or_dec == DEC_NUM){
		scanw("%d", input_num);
	// 16進数ならば
	}else{
		scanw("%x", input_num);
	}
	curs_set(0);
	//--------------------------------------
	// 入力値を設定画面データに設定
	//--------------------------------------
	set_scr->input_num[item_num].set_num = *input_num;
	//--------------------------------------
	// メッセージエリアクリア
	//--------------------------------------
	wclear(term_data.set_window.pmessage_window);
	//--------------------------------------
	// 設定情報表示
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	
	return TERM_ERR_SUCCESS;
}

