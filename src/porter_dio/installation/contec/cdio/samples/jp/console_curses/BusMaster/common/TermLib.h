//================================================================
//================================================================
// CONTEC Linux BusMaster Sample for DIO
//			Terminal Operation Library Header File
//													CONTEC Co.,Ltd.
//													Ver1.00
//================================================================
//================================================================
#ifndef __CDM_TERM_LIB_H_DEFINED__
#define __CDM_TERM_LIB_H_DEFINED__

#include <curses.h>

//=================================================
// マクロ定義
//=================================================
#define	DEC_NUM					0	// 10進数
#define	HEX_NUM					1	// 16進数
#define	MAIN_AREA				0	// メインウインドウ
#define	SET_AREA				1	// 設定ウインドウ
#define	STATUS_AREA_TOP			0	// 上部ステータスエリア
#define	STATUS_AREA_BOTTOM		1	// 下部ステータスエリア
#define	MENU_NUM				10	// メニュー個数
#define	MENU_TIER				5	// メニュー階層
#define	STATUS_NUM				5	// ステータス表示最大数
#define	SET_ITEM_MAX_COUNT		20	// 最大設定項目数
#define SELECT_ITEM_MAX_COUNT	30	// 最大選択肢

//=================================================
// エラー定義
//=================================================
#define	TERM_ERR_SUCCESS		0	// 正常終了
#define	TERM_ERR_INIT			1	// 初期化エラー
#define	TERM_ERR_UN_INIT		2	// 未初期化エラー
#define	TERM_ERR_PARAM			3	// パラメータエラー
#define	TERM_ERR_INIT_SCR		4	// ウィンドウ生成失敗
#define	TERM_ERR_DUP_EXEC		5	// 重複実行エラー

//=================================================
// 構造体
//=================================================
//=================================================
// メイン画面データ構造体
//=================================================
//-------------------------
//	メニューエリアデータ
//-------------------------
typedef struct{
	char name[256];				// メニュー名
}MENU_ITEM, *PMENU_ITEM;
//-------------------------
//	ステータスエリアデータ
//-------------------------
typedef struct{
	char label[256];			// ステータス項目名
}STATUS_SCR;
//-------------------------
//	メイン画面データ
//-------------------------
typedef struct{
	char		app_name[256];							// アプリケーション名
	char		status_top_name[256];					// 上部ステータスエリア名
	char		status_buttom_name[256];				// 下部ステータスエリア名
	MENU_ITEM	menu[MENU_TIER][MENU_NUM];				// メニュー[階層][メニュー個数]
	STATUS_SCR	status_top[STATUS_NUM];					// 上部ステータスエリアデータ
	STATUS_SCR	status_buttom[STATUS_NUM];				// 下部ステータスエリアデータ
}MAIN_SCR_DATA, *PMAIN_SCR_DATA;

//=================================================
// 設定画面データ構造体
//=================================================
//-------------------------
//	項目選択タイプ用設定データ
//-------------------------
typedef struct{
	int		pos_y;										// 設定値表示画面で何段目に表示するか？
	char	set_item_name[256];							// 項目名
	struct{
		char			name[256];						// 選択肢
		unsigned long	num;							// メニューに対応するマクロ
	}item[SELECT_ITEM_MAX_COUNT];
	unsigned long	set_num;							// 設定値(何番目の選択肢か)
}SELECT_ITEM_DATA;
//-------------------------
//	数値入力タイプ用設定データ
//-------------------------
typedef struct{
	int		pos_y;										// 設定値表示画面で何段目に表示するか？
	char	set_item_name[256];							// 項目名
	char	unit_name[256];								// 単位名
	long	set_num;									// 設定値
	int		hex_or_dec;									// 何進数か？ HEX_NUM or DEC_NUM
}INPUT_NUM_DATA;
//-------------------------
//	設定画面データ
//-------------------------
typedef struct{
	char				window_name[256];				// 設定画面名
	MENU_ITEM			menu[MENU_TIER][MENU_NUM];		// メニュー[階層][メニュー個数]
	SELECT_ITEM_DATA	select_item[SET_ITEM_MAX_COUNT];// 項目選択タイプ設定データ
	INPUT_NUM_DATA		input_num[SET_ITEM_MAX_COUNT];	// 数値入力タイプ設定データ
}SET_SCR_DATA, *PSET_SCR_DATA;

//=================================================
// TermLibデータ構造体
//=================================================
//-------------------------
//	ウインドウデータ
//-------------------------
typedef struct{
	WINDOW	*pmain_window;				// メイン
	WINDOW	*pcap_window;				// キャプション
	WINDOW	*pmenu_window;				// メニュー
	WINDOW	*pmessage_window;			// メッセージ
	WINDOW	*psts_top_window;			// 上部ステータス
	WINDOW	*psts_bottom_window;		// 下部ステータス
	WINDOW	*pset_info_window;			// 情報表示
	short	menu_tier;					// メニューの階層数
}WINDOW_DATA, *PWINDOW_DATA;
//-------------------------
//	ライブラリ設定データ
//-------------------------
typedef struct{
	WINDOW		*std_window;			// 標準ウインドウ
	WINDOW_DATA	main_window;			// メインウインドウデータ
	WINDOW_DATA	set_window;				// 設定ウインドウデータ
	WINDOW_DATA	*active_window;			// アクティブなウインドウデータへのポインタ 
	void		(*org_resize)(int);		// 端末サイズ更新 curses側シグナル処理関数
	int			sem_id;					// セマフォ識別子
}TERM_DATA, *PTERM_DATA;

//================================================================
// 関数プロトタイプ
//================================================================
#ifdef __cplusplus
extern"C"{
#endif
	long TermInit(PMAIN_SCR_DATA main_scr);
	long TermExit(void);
	long TermMenuSet(PMENU_ITEM menu);
	long TermGetMenuNum(char *message, int *menu_num);
	long TermGetStr(char *message, char *get_str);
	long TermSetStatus(int sts_area, int pos_y, char *status_str);
	long TermSetWindowOpen(PSET_SCR_DATA set_scr);
	long TermSetWindowClose(void);
	long TermGetSelectItem(PSET_SCR_DATA set_scr, int item_num, unsigned long *select_num);
	long TermGetInputNum(PSET_SCR_DATA set_scr, int item_num, unsigned long *input_num);
#ifdef __cplusplus
}
#endif
#endif
