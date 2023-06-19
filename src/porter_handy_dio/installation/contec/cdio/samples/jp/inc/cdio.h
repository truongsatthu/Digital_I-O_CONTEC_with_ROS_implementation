//================================================================
//================================================================
// CONTEC Linux DIO ヘッダーファイル
//                  					CONTEC Co.,Ltd.
//										Ver2.20
//================================================================
//================================================================
#ifndef __CDIO_H_DEFINED__
#define __CDIO_H_DEFINED__

//================================================================
// マクロ定義
//================================================================
// コールバック関数の形式
typedef void (*PDIO_INT_CALLBACK)(short, short, long, long, void *);
typedef void (*PDIO_TRG_CALLBACK)(short, short, long, long, void *);
typedef void (*PDIO_STOP_CALLBACK)(short, short, long, void *);
typedef void (*PDIO_COUNT_CALLBACK)(short, short, long, void *);
// メッセージ
#define	DIOM_INTERRUPT				0x1300
#define	DIOM_TRIGGER				0x1340
#define	DIO_DMM_STOP				0x1400
#define	DIO_DMM_COUNT				0x1440
//================================================================
// エラーコード
//================================================================
#define	DIO_ERR_SUCCESS					0		// 正常終了
#define	DIO_ERR_INI_RESOURCE			1		// リソースの取得に失敗しました。
#define	DIO_ERR_INI_INTERRUPT			2		// 割り込みルーチンの登録に失敗しました。
#define	DIO_ERR_INI_MEMORY				3		// メモリの割り当てに失敗しました。このエラーは殆ど発生しません。もし発生した場合は、メモリを増設してみてください。
#define	DIO_ERR_INI_REGISTRY			4		// 設定ファイルのアクセスに失敗しました。
#define	DIO_ERR_DLL_DEVICE_NAME			10000	// 設定ファイルに登録されていないデバイス名が指定されました。
#define	DIO_ERR_DLL_INVALID_ID			10001	// 無効なIDが指定されました。初期化関数が正常に終了しているかを確認してください。また、IDを格納する変数のスコープを確認してください。
#define	DIO_ERR_DLL_CALL_DRIVER			10002	// ドライバを呼び出せません(ioctlに失敗)。
#define	DIO_ERR_DLL_CREATE_FILE			10003	// ファイルの作成に失敗しました(open失敗)。
#define	DIO_ERR_DLL_CLOSE_FILE			10004	// ファイルのクローズに失敗しました(close失敗)。
#define	DIO_ERR_DLL_CREATE_THREAD		10005	// スレッドの作成に失敗しました。
#define	DIO_ERR_INFO_INVALID_DEVICE		10050	// 指定したデバイス名称が見つかりません。スペルを確認してください。
#define	DIO_ERR_INFO_NOT_FIND_DEVICE	10051	// 利用可能なデバイスが見つかりません。
#define	DIO_ERR_INFO_INVALID_INFOTYPE	10052	// 指定したデバイス情報タイプが範囲外です。
#define	DIO_ERR_DLL_BUFF_ADDRESS		10100	// データバッファアドレスが不正です。
#define	DIO_ERR_DLL_TRG_KIND			10300	// トリガ種類が指定範囲外です。
#define	DIO_ERR_DLL_CALLBACK			10400	// コールバック関数のアドレスが無効です。
#define DIO_ERR_DLL_DIRECTION			10500	// 入出力方向が設定範囲外です。
#define	DIO_ERR_SYS_MEMORY				20000	// メモリの確保に失敗しました。このエラーは殆ど発生しません。もし発生した場合は、メモリを増設してみてください。
#define	DIO_ERR_SYS_NOT_SUPPORTED		20001	// このデバイスではこの関数は使用できません。
#define	DIO_ERR_SYS_BOARD_EXECUTING		20002	// デバイスが動作中のため実行できません。
#define	DIO_ERR_SYS_USING_OTHER_PROCESS	20003	// 他のプロセスがデバイスを使用しているため、実行できません。
#define	DIO_ERR_SYS_NOT_SUPPORT_KERNEL	20004	// 対応外のカーネルです。
#define	DIO_ERR_SYS_PORT_NO				20100	// ポート番号が指定可能範囲を超えています。
#define	DIO_ERR_SYS_PORT_NUM			20101	// ポート数が指定可能範囲を超えています。
#define	DIO_ERR_SYS_BIT_NO				20102	// ビット番号が指定可能範囲を超えています。
#define	DIO_ERR_SYS_BIT_NUM				20103	// ビット数が指定可能範囲を超えています。
#define	DIO_ERR_SYS_BIT_DATA			20104	// ビットデータが0か1以外です。
#define	DIO_ERR_SYS_INT_BIT				20200	// 割り込みビットが指定可能範囲外です。
#define	DIO_ERR_SYS_INT_LOGIC			20201	// 割り込み論理が指定可能範囲外です。
#define	DIO_ERR_SYS_TRG_LOGIC			20202	// トリガ論理が指定可能範囲外です。
#define	DIO_ERR_SYS_TIM					20300	// タイマ値が指定範囲外です。トリガ関数でのエラーです。
#define	DIO_ERR_SYS_FILTER				20400	// フィルタ時定数が指定範囲外です。
#define	DIO_ERR_SYS_8255				20500	// 8255チップ番号が指定可能範囲外です。
#define DIO_ERR_SYS_DIRECTION			50000	// 入出力方向が設定範囲外です。
#define DIO_ERR_SYS_SIGNAL				50001	// 使用できる信号の範囲外です。
#define DIO_ERR_SYS_START				50002	// 使用できるスタート条件の範囲外です。
#define DIO_ERR_SYS_CLOCK				50003	// クロック条件が設定範囲外です。
#define DIO_ERR_SYS_CLOCK_VAL			50004	// クロック値が設定範囲外です。
#define DIO_ERR_SYS_CLOCK_UNIT			50005	// クロック値単位が設定範囲外です。
#define DIO_ERR_SYS_STOP				50006	// ストップ条件が設定範囲外です。
#define DIO_ERR_SYS_STOP_NUM			50007	// 停止個数が設定範囲外です。
#define DIO_ERR_SYS_RESET				50008	// リセット内容が設定範囲外です。
#define DIO_ERR_SYS_LEN					50009	// データ個数が設定範囲外です。
#define DIO_ERR_SYS_RING				50010	// バッファ繰り返し使用設定が設定範囲外です。
#define DIO_ERR_SYS_COUNT				50011	// データ転送個数が設定範囲外です。
#define DIO_ERR_DM_BUFFER				50100	// バッファが大きすぎて確保できませんでした。
#define DIO_ERR_DM_LOCK_MEMORY			50101	// メモリがロックできませんでした。
#define DIO_ERR_DM_PARAM				50102	// パラメータエラー
#define DIO_ERR_DM_SEQUENCE				50103	// 実行手順エラー
//================================================================
// 情報タイプ
//================================================================
#define	IDIO_DEVICE_TYPE		0		// デバイスタイプ(short)
#define	IDIO_NUMBER_OF_8255		1		// 8255数(int)
#define	IDIO_IS_8255_BOARD		2		// 8255タイプ(int)
#define	IDIO_NUMBER_OF_DI_BIT	3		// DI BIT(short)
#define	IDIO_NUMBER_OF_DO_BIT	4		// DO BIT(short)
#define	IDIO_NUMBER_OF_DI_PORT	5		// DI PORT(short)
#define	IDIO_NUMBER_OF_DO_PORT	6		// DO PORT(short)
#define	IDIO_IS_POSITIVE_LOGIC	7		// 正論理?(int)
#define	IDIO_IS_ECHO_BACK		8		// エコーバック可能?(int)
#define	IDIO_NUM_OF_PORT		9		// 占有ポート数(short)
//================================================================
// 割り込み、トリガ立上り、立ち下がり
//================================================================
#define	DIO_INT_NONE			0		// 割り込み：マスク
#define	DIO_INT_RISE			1		// 割り込み：立上り
#define	DIO_INT_FALL			2		// 割り込み：立ち下がり
#define	DIO_TRG_NONE			0		// トリガ：マスク
#define	DIO_TRG_RISE			1		// トリガ：立上り
#define	DIO_TRG_FALL			2		// トリガ：立ち下がり
//================================================================
// デバイスタイプ
//================================================================
#define	DEVICE_TYPE_ISA			0		//ISAまたはCバス
#define	DEVICE_TYPE_PCI			1		//PCIバス
#define	DEVICE_TYPE_PCMCIA		2		//PCMCIA
#define	DEVICE_TYPE_USB			3		//USB
#define	DEVICE_TYPE_FIT			4		//FIT
//================================================================
// Direction
//================================================================
#define PI_32					1		//32点入力
#define PO_32					2		//32点出力
#define PIO_1616				3		//16点入力、16点出力
#define DIODM_DIR_IN			0x1		//入力
#define DIODM_DIR_OUT			0x2		//出力
//================================================================
// Start
//================================================================
#define DIODM_START_SOFT		1		//ソフトスタート
#define DIODM_START_EXT_RISE	2		//外部トリガ立ち上がり
#define DIODM_START_EXT_FALL	3		//外部トリガ立ち下り
#define DIODM_START_PATTERN		4		//パターン一致(入力のみ）
#define DIODM_START_EXTSIG_1	5		//SCコネクタEXTSIG1
#define DIODM_START_EXTSIG_2	6		//SCコネクタEXTSIG2
#define DIODM_START_EXTSIG_3	7		//SCコネクタEXTSIG3
//================================================================
// Clock
//================================================================
#define DIODM_CLK_CLOCK			1		//内部クロック(タイマー）
#define DIODM_CLK_EXT_TRG		2		//外部トリガ
#define DIODM_CLK_HANDSHAKE		3		//ハンドシェーク
#define DIODM_CLK_EXTSIG_1		4		//SCコネクタEXTSIG1
#define DIODM_CLK_EXTSIG_2		5		//SCコネクタEXTSIG2
#define DIODM_CLK_EXTSIG_3		6		//SCコネクタEXTSIG3
//================================================================
// Internal Clock
//================================================================
#define	DIODM_TIM_UNIT_S		1		//1秒単位
#define	DIODM_TIM_UNIT_MS		2		//1ms単位
#define	DIODM_TIM_UNIT_US		3		//1μs単位
#define	DIODM_TIM_UNIT_NS		4		//1ns単位
//================================================================
// Stop
//================================================================
#define DIODM_STOP_SOFT			1		//ソフトストップ
#define DIODM_STOP_EXT_RISE		2		//外部トリガ立ち上がり
#define DIODM_STOP_EXT_FALL		3		//外部トリガ立ち下り
#define DIODM_STOP_NUM			4		//指定個数で転送完了
#define DIODM_STOP_EXTSIG_1		5		//SCコネクタEXTSIG1
#define DIODM_STOP_EXTSIG_2		6		//SCコネクタEXTSIG2
#define DIODM_STOP_EXTSIG_3		7		//SCコネクタEXTSIG3
//================================================================
// ExtSig
//================================================================
#define DIODM_EXT_START_SOFT_IN			1		//ソフトスタート(パターン入力）
#define DIODM_EXT_STOP_SOFT_IN			2		//ソフトストップ(パターン入力）
#define DIODM_EXT_CLOCK_IN				3		//内部クロック(パターン入力）
#define DIODM_EXT_EXT_TRG_IN			4		//外部クロック(パターン入力）
#define DIODM_EXT_START_EXT_RISE_IN		5		//外部スタートトリガ立上り(パターン入力）
#define DIODM_EXT_START_EXT_FALL_IN		6		//外部スタートトリガ立下り(パターン入力）
#define DIODM_EXT_START_PATTERN_IN		7		//パターン一致(パターン入力）
#define DIODM_EXT_STOP_EXT_RISE_IN		8		//外部スタートトリガ立上り(パターン入力）
#define DIODM_EXT_STOP_EXT_FALL_IN		9		//外部スタートトリガ立下り(パターン入力）
#define DIODM_EXT_CLOCK_ERROR_IN		10		//クロックエラー(パターン入力）
#define DIODM_EXT_HANDSHAKE_IN			11		//ハンドシェイク(パターン入力）
#define	DIODM_EXT_TRNSNUM_IN			12		//指定個数で転送完了(パターン入力）
	
#define DIODM_EXT_START_SOFT_OUT		101		//ソフトスタート(パターン出力）
#define DIODM_EXT_STOP_SOFT_OUT			102		//ソフトストップ(パターン出力）
#define DIODM_EXT_CLOCK_OUT				103		//内部クロック(パターン出力）
#define DIODM_EXT_EXT_TRG_OUT			104		//外部クロック(パターン出力）
#define DIODM_EXT_START_EXT_RISE_OUT	105		//外部スタートトリガ立上り(パターン出力）
#define DIODM_EXT_START_EXT_FALL_OUT	106		//外部スタートトリガ立下り(パターン出力）
#define DIODM_EXT_STOP_EXT_RISE_OUT		107		//外部スタートトリガ立上り(パターン出力）
#define DIODM_EXT_STOP_EXT_FALL_OUT		108		//外部スタートトリガ立下り(パターン出力）
#define DIODM_EXT_CLOCK_ERROR_OUT		109		//クロックエラー(パターン出力）
#define DIODM_EXT_HANDSHAKE_OUT			110		//ハンドシェイク(パターン出力）
//================================================================
// Status
//================================================================
#define DIODM_STATUS_BMSTOP				0x1		//バスマスタ転送が完了したことを示します。
#define DIODM_STATUS_PIOSTART			0x2		//PIO入出力がスタートしたことを示します。
#define DIODM_STATUS_PIOSTOP			0x4		//PIO入出力がストップしたことを示します。
#define DIODM_STATUS_TRGIN				0x8		//外部スタートで、スタート信号が入ったことを示します。
#define DIODM_STATUS_OVERRUN			0x10	//外部スタートで、スタート信号が２回以上入ったことを示します。
//================================================================
// Error
//================================================================
#define DIODM_STATUS_FIFOEMPTY			0x1		//出力でFIFOが空になったことを示します。
#define DIODM_STATUS_FIFOFULL			0x2		//入力でFIFOが一杯になったことを示します。
#define DIODM_STATUS_SGOVERIN			0x4		//バッファがオーバーフローしたことを示します。
#define DIODM_STATUS_TRGERR				0x8		//外部スタートで、スタート信号とストップ信号が同時に入ったことを示します。
#define DIODM_STATUS_CLKERR				0x10	//外部クロックで、データ入出力中に次のクロックが入ったことを示します。
#define DIODM_STATUS_SLAVEHALT			0x20
#define DIODM_STATUS_MASTERHALT			0x40
//================================================================
// Reset
//================================================================
#define	DIODM_RESET_FIFO_IN				0x02	//入力のFIFOをリセット
#define	DIODM_RESET_FIFO_OUT			0x04	//出力のFIFOをリセット
//================================================================
// Buffer Ring
//================================================================
#define	DIODM_WRITE_ONCE				0		//1回転送
#define	DIODM_WRITE_RING				1		//無限回転送
//================================================================
// 関数プロトタイプ
//================================================================
#ifdef __cplusplus
extern"C"{
#endif
long DioInit(char *device_name, short *id);
long DioExit(short id);
long DioResetDevice(short id);
long DioGetErrorString(long err_code, char *err_string);
long DioSetDigitalFilter(short id, short filter_value);
long DioSetIoDirection(short id, long dir);
long DioGetIoDirection(short id, long *dir);
long DioSet8255Mode(short id, unsigned short chip_no, unsigned short ctrl_word);
long DioGet8255Mode(short id, unsigned short chip_no, unsigned short *ctrl_word);
long DioInpByte(short id, short port_no, unsigned char *data);
long DioInpByteSR(short id, short port_no, unsigned char *data, unsigned short *time_stamp, unsigned char mode);
long DioInpBit(short id, short bit_no, unsigned char *data);
long DioInpBitSR(short id, short bit_no, unsigned char *data, unsigned short *time_stamp, unsigned char mode);
long DioOutByte(short id, short port_no, unsigned char data);
long DioOutBit(short id, short bit_no, unsigned char data);
long DioEchoBackByte(short id, short port_no, unsigned char *data);
long DioEchoBackBit(short id, short bit_no, unsigned char *data);
long DioInpMultiByte(short id, short *port_no, short port_num, unsigned char *data);
long DioInpMultiByteSR(short id, short *port_no, short port_num, unsigned char *data, unsigned short *time_stamp, unsigned char mode);
long DioInpMultiBit(short id, short *bit_no, short bit_num, unsigned char *data);
long DioInpMultiBitSR(short id, short *bit_no, short bit_num, unsigned char *data, unsigned short *time_stamp, unsigned char mode);
long DioOutMultiByte(short id, short *port_no, short port_num, unsigned char *data);
long DioOutMultiBit(short id, short *bit_no, short bit_num, unsigned char *data);
long DioEchoBackMultiByte(short id, short *port_no, short port_num, unsigned char *data);
long DioEchoBackMultiBit(short id, short *bit_no, short bit_num, unsigned char *data);
long DioSetInterruptEvent(short id, short bit_no, short logic);
long DioSetInterruptCallBackProc(short id, PDIO_INT_CALLBACK call_back, void *param);
long DioSetTrgEvent(short id, short bit_no, short logic, long tim);
long DioSetTrgCallBackProc(short id, PDIO_TRG_CALLBACK call_back, void *param);
long DioQueryDeviceName(short index, char *device_name, char *device);
long DioGetDeviceInfo(char *device, short info_type, void *param1, void *param2, void *param3);
long DioGetMaxPorts(short id, short *in_port_num, short *out_port_num);
long DioGetPatternEventStatus(short id, short *status);
long DioResetPatternEvent(short id, unsigned char *data);
long DioDmSetDirection(short id, unsigned long Direction);
long DioDmSetStandAlone(short id);
long DioDmSetMasterCfg(short id, unsigned long ExtSig1, unsigned long ExtSig2, unsigned long ExtSig3,
									unsigned long MasterHalt, unsigned long SlaveHalt);
long DioDmSetSlaveCfg(short id, unsigned long ExtSig1, unsigned long ExtSig2, unsigned long ExtSig3,
									unsigned long MasterHalt, unsigned long SlaveHalt);
long DioDmSetStartTrg(short id, unsigned long Dir, unsigned long Start);
long DioDmSetStartPattern(short id, unsigned long Ptn, unsigned long Mask);
long DioDmSetClockTrg(short id, unsigned long Dir, unsigned long Clock);
long DioDmSetInternalClock(short id, unsigned long Dir, unsigned long Clock, unsigned long Unit);
long DioDmSetStopTrg(short id, unsigned long Dir, unsigned long Stop);
long DioDmSetStopNum(short id, unsigned long Dir, unsigned long StopNum);
long DioDmReset(short id, unsigned long Reset);
long DioDmSetBuff(short id, unsigned long Dir, unsigned long *Buff, unsigned long Len, unsigned long IsRing);
long DioDmStart(short id, unsigned long Dir);
long DioDmStop(short id, unsigned long Dir);
long DioDmGetStatus(short id, unsigned long Dir, unsigned long *Status, unsigned long *Err);
long DioDmGetCount(short id, unsigned long Dir, unsigned long *Count, unsigned long *Carry);
long DioDmGetWritePointerUserBuf(short id, unsigned long Dir, unsigned long *WritePointer, unsigned long *Count, unsigned long *Carry);
long DioDmGetFifoCount(short id, unsigned long Dir, unsigned long *Count);
long DioDmSetStopEvent(short id, unsigned long Dir, PDIO_STOP_CALLBACK CallBack, void *Parameter);
long DioDmSetCountEvent(short id, unsigned long Dir, unsigned long Count, PDIO_COUNT_CALLBACK CallBack, void *Parameter);
#ifdef __cplusplus
}
#endif
#endif


