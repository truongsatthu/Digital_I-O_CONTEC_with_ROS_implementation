//================================================================
//================================================================
// CONTEC Linux DIO Header File
//                  					CONTEC Co.,Ltd.
//										Ver2.20
//================================================================
//================================================================
#ifndef __CDIO_H_DEFINED__
#define __CDIO_H_DEFINED__

//================================================================
// Define macro
//================================================================
// Form of callback function
typedef void (*PDIO_INT_CALLBACK)(short, short, long, long, void *);
typedef void (*PDIO_TRG_CALLBACK)(short, short, long, long, void *);
typedef void (*PDIO_STOP_CALLBACK)(short, short, long, void *);
typedef void (*PDIO_COUNT_CALLBACK)(short, short, long, void *);
// Message
#define	DIOM_INTERRUPT				0x1300
#define	DIOM_TRIGGER				0x1340
#define	DIO_DMM_STOP				0x1400
#define	DIO_DMM_COUNT				0x1440
//================================================================
// Error code
//================================================================
#define	DIO_ERR_SUCCESS					0		// Normal complete
#define	DIO_ERR_INI_RESOURCE			1		// Failed in the acquisition of resource.
#define	DIO_ERR_INI_INTERRUPT			2		// Failed in the registration of interrupt routine.
#define	DIO_ERR_INI_MEMORY				3		// Failed in the memory allocation. This error hardly occurs. If this error has occurred, please install more memory.
#define	DIO_ERR_INI_REGISTRY			4		// Failed in accessing the setting file.
#define	DIO_ERR_DLL_DEVICE_NAME			10000	// Device name which isn't registered in setting file is specified.
#define	DIO_ERR_DLL_INVALID_ID			10001	// Invalid ID is specified. Make sure whether initialization function normally complete. And, make sure the scope of variable in which ID is stored.
#define	DIO_ERR_DLL_CALL_DRIVER			10002	// Driver cannot be called (failed in ioctl).
#define	DIO_ERR_DLL_CREATE_FILE			10003	// Failed in creating file (open failed).
#define	DIO_ERR_DLL_CLOSE_FILE			10004	// Failed in closing file (close failed).
#define	DIO_ERR_DLL_CREATE_THREAD		10005	// Failed in creating thread.
#define	DIO_ERR_INFO_INVALID_DEVICE		10050	// Specified device name isn't found. Please check the spelling.
#define	DIO_ERR_INFO_NOT_FIND_DEVICE	10051	// The usable device isn't found.
#define	DIO_ERR_INFO_INVALID_INFOTYPE	10052	// The specified device information type is outside the range.
#define	DIO_ERR_DLL_BUFF_ADDRESS		10100	// Invalid data buffer address.
#define	DIO_ERR_DLL_TRG_KIND			10300	// Trigger type is outside the range.
#define	DIO_ERR_DLL_CALLBACK			10400	// Invalid address of callback function.
#define DIO_ERR_DLL_DIRECTION			10500	// I/O direction is outside the setting range.
#define	DIO_ERR_SYS_MEMORY				20000	// Failed in memory. This error hardly occurs. If this error has occurred, please install more memory.
#define	DIO_ERR_SYS_NOT_SUPPORTED		20001	// This function cannot be used in this device.
#define	DIO_ERR_SYS_BOARD_EXECUTING		20002	// It cannot perform because the device is executing.
#define	DIO_ERR_SYS_USING_OTHER_PROCESS	20003	// It cannot perform because the other process is using the device.
#define	DIO_ERR_SYS_NOT_SUPPORT_KERNEL	20004	// It is not supporting in the kernel of this version.
#define	DIO_ERR_SYS_PORT_NO				20100	// PortNo is outside the setting range.
#define	DIO_ERR_SYS_PORT_NUM			20101	// Number of ports is outside the setting range.
#define	DIO_ERR_SYS_BIT_NO				20102	// BitNo is outside the range.
#define	DIO_ERR_SYS_BIT_NUM				20103	// Number of bits is outside the setting range.
#define	DIO_ERR_SYS_BIT_DATA			20104	// Bit data is not 0 or 1.
#define	DIO_ERR_SYS_INT_BIT				20200	// Interrupt bit is outside the setting range.
#define	DIO_ERR_SYS_INT_LOGIC			20201	// Interrupt logic is outside the setting range.
#define	DIO_ERR_SYS_TRG_LOGIC			20202	// Trigger logic is outside the setting range.
#define	DIO_ERR_SYS_TIM					20300	// Timer value is outside the setting range. Error in trigger function.
#define	DIO_ERR_SYS_FILTER				20400	// Filter value is outside the setting range.
#define	DIO_ERR_SYS_8255				20500	// 8255 chip number is output the setting range
#define DIO_ERR_SYS_DIRECTION			50000	// I/O direction is outside the setting range.
#define DIO_ERR_SYS_SIGNAL				50001	// Usable signal is outside the setting range.
#define DIO_ERR_SYS_START				50002	// Usable start conditions are outside the setting range.
#define DIO_ERR_SYS_CLOCK				50003	// Clock conditions are outside the setting range.
#define DIO_ERR_SYS_CLOCK_VAL			50004	// Clock value is outside the setting range.
#define DIO_ERR_SYS_CLOCK_UNIT			50005	// Clock value unit is outside the setting range.
#define DIO_ERR_SYS_STOP				50006	// Stop conditions are outside the setting range.
#define DIO_ERR_SYS_STOP_NUM			50007	// Stop number is outside the setting range.
#define DIO_ERR_SYS_RESET				50008	// Contents of reset are outside the setting range.
#define DIO_ERR_SYS_LEN					50009	// Data number is outside the setting range.
#define DIO_ERR_SYS_RING				50010	// Buffer repetition use setup is outside the setting range.
#define DIO_ERR_SYS_COUNT				50011	// Data transmission number is outside the setting range.
#define DIO_ERR_DM_BUFFER				50100	// Buffer was too large and has not secured.
#define DIO_ERR_DM_LOCK_MEMORY			50101	// Memory has not been locked.
#define DIO_ERR_DM_PARAM				50102	// Parameter error
#define DIO_ERR_DM_SEQUENCE				50103	// Procedure error of execution
//================================================================
// Information type
//================================================================
#define	IDIO_DEVICE_TYPE		0		// Device type(short)
#define	IDIO_NUMBER_OF_8255		1		// 8255 number(int)
#define	IDIO_IS_8255_BOARD		2		// 8255 type(int)
#define	IDIO_NUMBER_OF_DI_BIT	3		// DI BIT(short)
#define	IDIO_NUMBER_OF_DO_BIT	4		// DO BIT(short)
#define	IDIO_NUMBER_OF_DI_PORT	5		// DI PORT(short)
#define	IDIO_NUMBER_OF_DO_PORT	6		// DO PORT(short)
#define	IDIO_IS_POSITIVE_LOGIC	7		// Positive logic(int)
#define	IDIO_IS_ECHO_BACK		8		// Echo back(int)
#define	IDIO_NUM_OF_PORT		9		// used number of ports(short)
//================================================================
// Interrupt, trigger rising, falling
//================================================================
#define	DIO_INT_NONE			0		// Interrupt:mask
#define	DIO_INT_RISE			1		// Interrupt:rising
#define	DIO_INT_FALL			2		// Interrupt:falling
#define	DIO_TRG_NONE			0		// Trigger:mask
#define	DIO_TRG_RISE			1		// Trigger:rising
#define	DIO_TRG_FALL			2		// Trigger:falling
//================================================================
// Device type
//================================================================
#define	DEVICE_TYPE_ISA			0		//ISA or C bus
#define	DEVICE_TYPE_PCI			1		//PCI bus
#define	DEVICE_TYPE_PCMCIA		2		//PCMCIA
#define	DEVICE_TYPE_USB			3		//USB
#define	DEVICE_TYPE_FIT			4		//FIT
//================================================================
// Direction
//================================================================
#define PI_32					1		//32-bit input
#define PO_32					2		//32-bit output
#define PIO_1616				3		//16-bit input, 16-bit output
#define DIODM_DIR_IN			0x1		//Input
#define DIODM_DIR_OUT			0x2		//Output
//================================================================
// Start
//================================================================
#define DIODM_START_SOFT		1		//Software start
#define DIODM_START_EXT_RISE	2		//External trigger rising
#define DIODM_START_EXT_FALL	3		//External trigger falling
#define DIODM_START_PATTERN		4		//Patter matching(only input)
#define DIODM_START_EXTSIG_1	5		//SC connector EXTSIG1
#define DIODM_START_EXTSIG_2	6		//SC connector EXTSIG2
#define DIODM_START_EXTSIG_3	7		//SC connector EXTSIG3
//================================================================
// Clock
//================================================================
#define DIODM_CLK_CLOCK			1		//Internal clock(timer)
#define DIODM_CLK_EXT_TRG		2		//External trigger
#define DIODM_CLK_HANDSHAKE		3		//Hand shake
#define DIODM_CLK_EXTSIG_1		4		//SC connector EXTSIG1
#define DIODM_CLK_EXTSIG_2		5		//SC connector EXTSIG2
#define DIODM_CLK_EXTSIG_3		6		//SC connector EXTSIG3
//================================================================
// Internal Clock
//================================================================
#define	DIODM_TIM_UNIT_S		1		//1s unit
#define	DIODM_TIM_UNIT_MS		2		//1ms unit
#define	DIODM_TIM_UNIT_US		3		//1us unit
#define	DIODM_TIM_UNIT_NS		4		//1ns unit
//================================================================
// Stop
//================================================================
#define DIODM_STOP_SOFT			1		//Software stop
#define DIODM_STOP_EXT_RISE		2		//External trigger rising
#define DIODM_STOP_EXT_FALL		3		//External trigger falling
#define DIODM_STOP_NUM			4		//Stop transfer on specified transfer number
#define DIODM_STOP_EXTSIG_1		5		//SC connector EXTSIG1
#define DIODM_STOP_EXTSIG_2		6		//SC connector EXTSIG2
#define DIODM_STOP_EXTSIG_3		7		//SC connector EXTSIG3
//================================================================
// ExtSig
//================================================================
#define DIODM_EXT_START_SOFT_IN			1		//Software start(pattern input)
#define DIODM_EXT_STOP_SOFT_IN			2		//Software stop(pattern input)
#define DIODM_EXT_CLOCK_IN				3		//Internal clock(pattern input)
#define DIODM_EXT_EXT_TRG_IN			4		//External clock(pattern input)
#define DIODM_EXT_START_EXT_RISE_IN		5		//External start trigger rising(pattern input)
#define DIODM_EXT_START_EXT_FALL_IN		6		//External start trigger falling(pattern input)
#define DIODM_EXT_START_PATTERN_IN		7		//Pattern matching(pattern input)
#define DIODM_EXT_STOP_EXT_RISE_IN		8		//External stop trigger rising(pattern input)
#define DIODM_EXT_STOP_EXT_FALL_IN		9		//External stop trigger falling(pattern input)
#define DIODM_EXT_CLOCK_ERROR_IN		10		//Clock error(pattern input)
#define DIODM_EXT_HANDSHAKE_IN			11		//Hand shake(pattern input)
#define	DIODM_EXT_TRNSNUM_IN			12		//Stop transfer on specified transfer number(pattern input)
	
#define DIODM_EXT_START_SOFT_OUT		101		//Software start(pattern output)
#define DIODM_EXT_STOP_SOFT_OUT			102		//Software stop(pattern output)
#define DIODM_EXT_CLOCK_OUT				103		//Internal clock(pattern output)
#define DIODM_EXT_EXT_TRG_OUT			104		//External clock(pattern output)
#define DIODM_EXT_START_EXT_RISE_OUT	105		//External start trigger rising(pattern output)
#define DIODM_EXT_START_EXT_FALL_OUT	106		//External start trigger falling(pattern output)
#define DIODM_EXT_STOP_EXT_RISE_OUT		107		//External stop trigger rising(pattern output)
#define DIODM_EXT_STOP_EXT_FALL_OUT		108		//External stop trigger falling(pattern output)
#define DIODM_EXT_CLOCK_ERROR_OUT		109		//Clock error(pattern output)
#define DIODM_EXT_HANDSHAKE_OUT			110		//Hand shake(pattern output)
//================================================================
// Status
//================================================================
#define DIODM_STATUS_BMSTOP				0x1		//Indicates that bus master transfer is complete.
#define DIODM_STATUS_PIOSTART			0x2		//Indicates that PIO input/output has started.
#define DIODM_STATUS_PIOSTOP			0x4		//Indicates that PIO input/ouput has stopped.
#define DIODM_STATUS_TRGIN				0x8		//Indicates that the start signal is inserted by external start.
#define DIODM_STATUS_OVERRUN			0x10	//Indicates that start signals are inserted twice or more by external start.
//================================================================
// Error
//================================================================
#define DIODM_STATUS_FIFOEMPTY			0x1		//Indicates that FIFO becomes vacant.
#define DIODM_STATUS_FIFOFULL			0x2		//Indicates that FIFO is full due to input.
#define DIODM_STATUS_SGOVERIN			0x4		//Indicates that the buffer has overflowed.
#define DIODM_STATUS_TRGERR				0x8		//Indicates that start and stop signals have been inserted simultaneously by external start.
#define DIODM_STATUS_CLKERR				0x10	//Indicates thar the next clock is inserted during data input/output by the external clock.
#define DIODM_STATUS_SLAVEHALT			0x20
#define DIODM_STATUS_MASTERHALT			0x40
//================================================================
// Reset
//================================================================
#define	DIODM_RESET_FIFO_IN				0x02	//Reset input FIFO
#define	DIODM_RESET_FIFO_OUT			0x04	//Reset output FIFO
//================================================================
// Buffer Ring
//================================================================
#define	DIODM_WRITE_ONCE				0		//Single transfer
#define	DIODM_WRITE_RING				1		//Unlimited transfer
//================================================================
// Function prototype
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



