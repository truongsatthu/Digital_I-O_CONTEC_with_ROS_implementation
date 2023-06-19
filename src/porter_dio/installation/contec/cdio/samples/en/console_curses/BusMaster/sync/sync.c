//================================================================
//================================================================
//	CONTEC Linux DIO (BusMaster)
//	Sync Sample
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
// Define macro
//================================================================
#define	MENU_MASTER 		0
#define	MENU_SLAVE  		1
#define	MENU_EXIT	 		2
#define	MENU_DIO_OPEN		0
#define	MENU_CONDITION		1
#define	MENU_START			2
#define	MENU_STOP			3
#define	MENU_DATA			4
#define	MENU_COUNT			5
#define	NENU_STATUS			6
#define	MENU_DIO_CLOSE		7
#define	MENU_RETURN			8
#define	SET_MENU_START		0
#define	SET_MENU_CLOCK		1
#define	SET_MENU_STOP		2
#define	SET_MENU_BUF_CON	3
#define	SET_MENU_NOTIF		4
#define	SET_MENU_OK			5
#define	SET_MENU_CANCEL		6
#define	STS_COUNT_POS		1
#define	STS_STATUS_POS		2
#define	STS_ERROR_POS		3
#define	STS_RET_POS			4
#define	SET_NUM_COMP_PTN	0
#define	SET_NUM_PTN_MASK	1
#define	SET_NUM_CLOCK		2
#define	SET_NUM_STOP_NUM	3
#define	SET_NUM_NOTIF		4

#define	SET_SLAVE_NUM_NOTIF	0

#define	DATA_SIZE_MASTER	1000					// Master data size
#define	DATA_SIZE_SLAVE		1000					// Slave data size
//================================================================
// External variable
//================================================================
static char				file_name_master[]	= "sync_master.txt";	// Name of master saving file
static char				file_name_slave[]	= "sync_slave.txt";		// Name of slave saving file
static unsigned long	data_buff_master[DATA_SIZE_MASTER];			// Master data buffer
static unsigned long	data_buff_slave[DATA_SIZE_SLAVE];			// Slave data buffer
short					id_master, id_slave;						// Device ID
unsigned long			is_ring_master	= DIODM_WRITE_ONCE;
unsigned long			is_ring_slave	= DIODM_WRITE_ONCE;
//-----------------------------------
// Main screen data
//-----------------------------------
MAIN_SCR_DATA	main_scr = 
{
	"<< Sync Sample >>",
	"[Master Status]",
	"[Slave Status]",
	{
		{
			{"Master"},{"Slave"},{"Exit"},{""}
		},
 		{
 			{"DioInit"},{"Condition..."},{"Start"},
			{"Stop"},{"Data"},{"Count"},{"Status"},{"DioExit"},{"Return"},
			{""}
 		},
 		{
 			{"DioInit"},{"Condition..."},{"Start"},
			{"Stop"},{"Data"},{"Count"},{"Status"},{"DioExit"},{"Return"},
			{""}
 		}
	},
	{{"Count"},{"Status"},{"Error"},{"Ret"},{""}},
	{{"Count"},{"Status"},{"Error"},{"Ret"},{""}}
};

//-----------------------------------
//	Setting master screen data
//-----------------------------------
SET_SCR_DATA	set_master_scr =
{
	"<< Master Sampling Condition >>",
	{
		{{"Start"},{"Clock"},{"Stop"},{"Buffer"},
		{"Notification"},{"OK"},{"Cancel"},
		{""},{""},{""}},
		{{""}}
	},
	{	// Setting data for type of selected item
		{
			0,
			"Start(ExtSig2)",

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
			"Clock(ExtSig1)",
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
			"Stop(ExtSig3)",
			{
				{"Software Stop", 	DIODM_STOP_SOFT},
				{"External UP",		DIODM_STOP_EXT_RISE},
				{"External DOWN",	DIODM_STOP_EXT_FALL},
				{"Number",			DIODM_STOP_NUM},
				{"", 0}
			},
			0
		},
		{
			4,
			"Buffer",
			{
				{"Write Once",      DIODM_WRITE_ONCE},
				{"Write Ring",      DIODM_WRITE_RING},
				{""}
			},
			0
		}
	},
	{	// Setting data for type of input numerical value
		{0, "Compare Pattern",	"Hex",	0x55,	HEX_NUM},
		{1, "Pattern Mask",		"Hex",	0xFF,	HEX_NUM},
		{2, "Internal Clock",	"ns",	1000,	DEC_NUM},
		{3, "Stop Number",		"",		1000,	DEC_NUM},
		{4, "Number of Data",	"",		500,	DEC_NUM}
	}
};

//-----------------------------------
//	Setting slave screen data
//-----------------------------------
SET_SCR_DATA	set_slave_scr = 
{
	"<< Slave Sampling Condition >>",
	{
		{{"Start"},{"Clock"},{"Stop"},{"Buffer"},
		{"Notification"},{"OK"},{"Cancel"},
		{""},{""},{""}},
		{{""}}
	},
	{	// Setting data for type of selected item
		{
			0,
			"Start",
			{
				{"ExtSig1",	DIODM_START_EXTSIG_1},
				{"ExtSig2",	DIODM_START_EXTSIG_2},
				{"ExtSig3",	DIODM_START_EXTSIG_3},
				{""}
			},
			1
		},
		{
			2,
			"Clock",
			{
				{"ExtSig1",	DIODM_CLK_EXTSIG_1},
				{"ExtSig2",	DIODM_CLK_EXTSIG_2},
				{"ExtSig3",	DIODM_CLK_EXTSIG_3},
				{""}
			},
			0
		},
		{
			3,
			"Stop",
			{
				{"ExtSig1", 	DIODM_STOP_EXTSIG_1},
				{"ExtSig2",		DIODM_STOP_EXTSIG_2},
				{"ExtSig3",     DIODM_STOP_EXTSIG_3},
				{"", 0}
			},
			2
		},
		{
			4,
			"Buffer",
			{
				{"Write Once",      DIODM_WRITE_ONCE},
				{"Write Ring",      DIODM_WRITE_RING},
				{""}
			},
			0
		}
	},
	{	//	Setting data for type of input numerical value
		{1, "Number of Data",	"",		500,	DEC_NUM}
	}
};

//================================================================
//	Master CallBack function of transfer complete
//================================================================
void stop_call_back_master(short id, short message, unsigned long dir, void *param)
{
	char buf[256];
	//------------------------------------------
	// Process interrupt message
	//------------------------------------------
	if (message == DIO_DMM_STOP){
		TermSetStatus(STATUS_AREA_TOP, STS_STATUS_POS, "Stopped!");
	//------------------------------------------
	// Others
	//------------------------------------------
	}else{
		sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
	}
	return;
}

//================================================================
//	Slave CallBack function of transfer complete
//================================================================
void stop_call_back_slave(short id, short message, unsigned long dir, void *param)
{
	char buf[256];
	//------------------------------------------
	// Process interrupt message
	//------------------------------------------
	if (message == DIO_DMM_STOP){
		TermSetStatus(STATUS_AREA_BOTTOM, STS_STATUS_POS, "Stopped!");
	//------------------------------------------
	// Others
	//------------------------------------------
	}else{
		sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
	}
	return;
}

//================================================================
//	Master CallBack function of transfer complete in specified number
//================================================================
void count_call_back_master(short id, short message, unsigned long dir, void *param)
{
	char			get_str[256], buf[256];
	unsigned long	count, carry;
	long			lret;
	//------------------------------------------
	// Process interrupt message
	//------------------------------------------
	if (message == DIO_DMM_COUNT){
		lret = DioDmGetCount(id, dir, &count, &carry);
		if(lret == TERM_ERR_SUCCESS){
			sprintf(buf, "%lu", count);
			TermSetStatus(STATUS_AREA_TOP, STS_COUNT_POS, buf);
		}
		DioGetErrorString(lret, get_str);
		sprintf(buf, "DioDmGetCount %ld:%s", lret, get_str);
		TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
	//------------------------------------------
	// Others
	//------------------------------------------
	}else{
		sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_TOP, STS_COUNT_POS, buf);
	}
	return;
}

//================================================================
//	Slave CallBack function of transfer complete in specified number
//================================================================
void count_call_back_slave(short id, short message, unsigned long dir, void *param)
{
	char 			get_str[256], buf[256];
	unsigned long	count, carry;
	long			lret;

	//------------------------------------------
	// Process interrupt message
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
	// Others
	//------------------------------------------
	}else{
		sprintf(buf, "Message Number : %d", message);
		TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, buf);
	}
	return;
}

//================================================================
//	Function for setting transfer condition (Master)
//================================================================
long master_condition(char *err_str)
{
	char			get_str[256];
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN;
	unsigned long	start, clock, stop;
	unsigned long	comp_ptn, ptn_mask, internal_clock, stop_num, notification;
	unsigned long	def_set_num;
	long            ExtSig1, ExtSig2, ExtSig3,i;
	long 			lret;

	// Setting data for external signal
	typedef struct {		
		char	Str[100];
		long	Num;
		long	ExtSig;
	} StrToNum;
	// Start Condition
	static	StrToNum stnStart[5] = {
		{	"Software Start",	DIODM_START_SOFT,		DIODM_EXT_START_SOFT_IN		},
		{	"External UP",		DIODM_START_EXT_RISE,	DIODM_EXT_START_EXT_RISE_IN	},
		{	"External Down",	DIODM_START_EXT_FALL,	DIODM_EXT_START_EXT_FALL_IN	},
		{	"Pattern",			DIODM_START_PATTERN,	DIODM_EXT_START_PATTERN_IN	},
		{	"End",				0,						0							}
	};
	// Clock Condition
	static	StrToNum stnClock[4] = {
		{	"Internal Clock",	DIODM_CLK_CLOCK,		DIODM_EXT_CLOCK_IN			},
		{	"External Clock",	DIODM_CLK_EXT_TRG,		DIODM_EXT_EXT_TRG_IN		},
		{	"Hand Shake",		DIODM_CLK_HANDSHAKE,	DIODM_EXT_HANDSHAKE_IN		},
		{	"End",				0,						0							}
	};
	// Stop Condition
	static	StrToNum stnStop[5] = {
		{	"Software Stop",	DIODM_STOP_SOFT,		DIODM_EXT_STOP_SOFT_IN		},
		{	"External UP",		DIODM_STOP_EXT_RISE,	DIODM_EXT_STOP_EXT_RISE_IN	},
		{	"External Down",	DIODM_STOP_EXT_FALL,	DIODM_EXT_STOP_EXT_FALL_IN	},
		{	"Number",			DIODM_STOP_NUM,			DIODM_EXT_TRNSNUM_IN        },
		{	"End",				0,						0							}
	}; 
	//-----------------------------------
	// Initialize setting data
	//-----------------------------------
	def_set_num		= set_master_scr.select_item[SET_MENU_START].set_num;
	start			= set_master_scr.select_item[SET_MENU_START].item[def_set_num].num;
	def_set_num		= set_master_scr.select_item[SET_MENU_CLOCK].set_num;
	clock			= set_master_scr.select_item[SET_MENU_CLOCK].item[def_set_num].num;
	def_set_num		= set_master_scr.select_item[SET_MENU_STOP].set_num;
	stop			= set_master_scr.select_item[SET_MENU_STOP].item[def_set_num].num;
	comp_ptn		= set_master_scr.input_num[SET_NUM_COMP_PTN].set_num;
	ptn_mask		= set_master_scr.input_num[SET_NUM_PTN_MASK].set_num;
	internal_clock	= set_master_scr.input_num[SET_NUM_CLOCK].set_num;
	stop_num		= set_master_scr.input_num[SET_NUM_STOP_NUM].set_num;
	notification	= set_master_scr.input_num[SET_NUM_NOTIF].set_num;
	//-----------------------------------
	// Initialize setting counter
	//-----------------------------------
	lret = TermSetWindowOpen(&set_master_scr);
	if(lret != TERM_ERR_SUCCESS){
		sprintf(err_str, "TermSetWindowOpen %ld\n", lret);
		return 1;
	}
	//-----------------------------------
	// Menu process
	//-----------------------------------
	strcpy(err_str, "");
	while(1){
		//-----------------------
		// Error occured in last round, close setting screen
		//-----------------------
		if(strcmp(err_str, "")){
			TermSetWindowClose();
			return 1;
		}
		//-----------------------
		// Input menu number
		//-----------------------
		lret = TermGetMenuNum("Please input a menu number(0-6).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			sprintf(err_str, "TermGetMenuNum %ld\n", lret);
			return 1;
		}
		switch(menu_num){
		//-----------------------------------
		// Set start condition
		//-----------------------------------
		case SET_MENU_START:
			lret = TermGetSelectItem(&set_master_scr, SET_MENU_START, &start);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(start == DIODM_START_PATTERN){
				lret = TermGetInputNum(&set_master_scr, SET_NUM_COMP_PTN, &comp_ptn);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}

				lret = TermGetInputNum(&set_master_scr, SET_NUM_PTN_MASK, &ptn_mask);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// Set clock condition
		//-----------------------------------
		case SET_MENU_CLOCK:
			lret = TermGetSelectItem(&set_master_scr, SET_MENU_CLOCK, &clock);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(clock == DIODM_CLK_CLOCK){
				lret = TermGetInputNum(&set_master_scr, SET_NUM_CLOCK, &internal_clock);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// Set stop condition
		//-----------------------------------
		case SET_MENU_STOP:
			lret = TermGetSelectItem(&set_master_scr, SET_MENU_STOP, &stop);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			if(stop == DIODM_STOP_NUM){
				lret = TermGetInputNum(&set_master_scr, SET_NUM_STOP_NUM, &stop_num);
				if(lret != TERM_ERR_SUCCESS){
					sprintf(err_str, "TermGetInputNum %ld\n", lret);
				}
			}
			break;
		//-----------------------------------
		// Set buffer condition
		//-----------------------------------
		case SET_MENU_BUF_CON:
			lret = TermGetSelectItem(&set_master_scr, SET_MENU_BUF_CON, &is_ring_master);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set information count
		//-----------------------------------
		case SET_MENU_NOTIF:
			lret = TermGetInputNum(&set_master_scr, SET_NUM_NOTIF, &notification);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetInputNum %ld\n", lret);
			}
			break;
		//-----------------------------------
		// OK menu process
		//-----------------------------------
		case SET_MENU_OK:
			// Direction
			lret = DioDmSetDirection(id_master, PI_32);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetDirection %ld:%s", lret, get_str);
				break;
			}
			// Master
			//--------------------------------
			// Parameter exchange
			//--------------------------------
			ExtSig2 = 0;
			for (i=0; strcmp(stnStart[i].Str, "End") != 0; i++) {
				if (start == stnStart[i].Num) {
					ExtSig2 = stnStart[i].ExtSig;
					break;
				}
			}
			ExtSig1 = 0;
			for (i=0; strcmp(stnClock[i].Str, "End") != 0; i++) {
				if (clock == stnClock[i].Num) {
					ExtSig1 = stnClock[i].ExtSig;
					break;
				}
			}
			ExtSig3 = 0;
			for (i=0; strcmp(stnStop[i].Str, "End") != 0; i++) {
				if (stop == stnStop[i].Num) {
					ExtSig3 = stnStop[i].ExtSig;
					break;
				}
			}            
			lret = DioDmSetMasterCfg(id_master,ExtSig1,ExtSig2,ExtSig3,1,1);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetMasterCfg %ld:%s", lret, get_str);
				break;
			}

			// Start Condition
			lret = DioDmSetStartTrg(id_master, dir, start);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStartTrg %ld:%s", lret, get_str);
				break;
			}
			// Clock Condition
			lret = DioDmSetClockTrg(id_master, dir, clock);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetClockTrg %ld:%s", lret, get_str);
				break;
			}
			// Stop Condition
			lret = DioDmSetStopTrg(id_master, dir, stop);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStopTrg %ld:%s", lret, get_str);
				break;
			}
			// Pattern/Mask
			lret = DioDmSetStartPattern(id_master, comp_ptn, ptn_mask);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStartPattern %ld:%s", lret, get_str);
				break;
			}
			// Internal Clock
			lret = DioDmSetInternalClock(id_master, dir, internal_clock, DIODM_TIM_UNIT_NS);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetInternalClock %ld:%s", lret, get_str);
				break;
			}
			// StopNumber
			lret = DioDmSetStopNum(id_master, dir, stop_num);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStopNum %ld:%s", lret, get_str);
				break;
			}
		//-----------------------------------
		// Cancel menu process
		//-----------------------------------
		case SET_MENU_CANCEL:
			//-----------------------------------
			// Close setting screen
			//-----------------------------------
			TermSetWindowClose();
			return 0;
			break;
		}
	}
	return 0;
}

//================================================================
//	Function for setting transfer condition (Slave)
//================================================================
long slave_condition(char *err_str)
{
	char			get_str[256];
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN;
	unsigned long	start, clock, stop;
	unsigned long	notification;
	unsigned long	def_set_num;
	long            ExtSig1, ExtSig2, ExtSig3;
	long 			lret;
	//-----------------------------------
	// Initialize serring data
	//-----------------------------------
	def_set_num		= set_slave_scr.select_item[SET_MENU_START].set_num;
	start			= set_slave_scr.select_item[SET_MENU_START].item[def_set_num].num;
	def_set_num		= set_slave_scr.select_item[SET_MENU_CLOCK].set_num;
	clock			= set_slave_scr.select_item[SET_MENU_CLOCK].item[def_set_num].num;
	def_set_num		= set_slave_scr.select_item[SET_MENU_STOP].set_num;
	stop			= set_slave_scr.select_item[SET_MENU_STOP].item[def_set_num].num;
	notification	= set_slave_scr.input_num[SET_SLAVE_NUM_NOTIF].set_num;
	//-----------------------------------
	// Initialize setting window
	//-----------------------------------
	lret = TermSetWindowOpen(&set_slave_scr);
	if(lret != TERM_ERR_SUCCESS){
		sprintf(err_str, "TermSetWindowOpen %ld\n", lret);
		return 1;
	}
	//-----------------------------------
	// Menu process
	//-----------------------------------
	strcpy(err_str, "");
	while(1){
		//-----------------------
		// Error occured in last round, close setting screen
		//-----------------------
		if(strcmp(err_str, "")){
			TermSetWindowClose();
			return 1;
		}
		//-----------------------
		// Input menu number
		//-----------------------
		lret = TermGetMenuNum("Please input a menu number(0-6).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			sprintf(err_str, "TermGetMenuNum %ld\n", lret);
			return 1;
		}
		switch(menu_num){
		//-----------------------------------
		// Set start condition
		//-----------------------------------
		case SET_MENU_START:
			lret = TermGetSelectItem(&set_slave_scr, SET_MENU_START, &start);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set clock condition
		//-----------------------------------
		case SET_MENU_CLOCK:
			lret = TermGetSelectItem(&set_slave_scr, SET_MENU_CLOCK, &clock);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set stop condition
		//-----------------------------------
		case SET_MENU_STOP:
			lret = TermGetSelectItem(&set_slave_scr, SET_MENU_STOP, &stop);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set buffer condition
		//-----------------------------------
		case SET_MENU_BUF_CON:
			lret = TermGetSelectItem(&set_slave_scr, SET_MENU_BUF_CON, &is_ring_slave);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set information count
		//-----------------------------------
		case SET_MENU_NOTIF:
			lret = TermGetInputNum(&set_slave_scr, SET_SLAVE_NUM_NOTIF, &notification);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetInputNum %ld\n", lret);
			}
			break;
		//-----------------------------------
		// OK menu process
		//-----------------------------------
		case SET_MENU_OK:
			// Direction
			lret = DioDmSetDirection(id_slave, PI_32);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetDirection %ld:%s", lret, get_str);
				break;
			}
			// Set external signal for slave
			ExtSig1 = 1;
			ExtSig2 = 1;
			ExtSig3 = 1;
			lret = DioDmSetSlaveCfg(id_slave, ExtSig1, ExtSig2, ExtSig3, 1, 1);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetSlaveCfg %ld:%s", lret, get_str);
				break;
			}
			// Start Condition
			lret = DioDmSetStartTrg(id_slave, dir, start);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStartTrg %ld:%s", lret, get_str);
				break;
			}
			// Clock Condition
			lret = DioDmSetClockTrg(id_slave, dir, clock);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetClockTrg %ld:%s", lret, get_str);
				break;
			}
			// Stop Condition
			lret = DioDmSetStopTrg(id_slave, dir, stop);
			if (lret != DIO_ERR_SUCCESS) {
				DioGetErrorString(lret, get_str);
				sprintf(err_str, "DioDmSetStopTrg %ld:%s", lret, get_str);
				break;
			}
		//-----------------------------------
		// Cancel menu process
		//-----------------------------------
		case SET_MENU_CANCEL:
			//-----------------------------------
			// Close setting screen
			//-----------------------------------
			TermSetWindowClose();
			return 0;
			break;
		}
	}
	return 0;
}

//================================================================
//	Master function
//================================================================
int master()
{
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN; 
	unsigned long	i, count, carry, status, err, notification;
	char			get_str[256], buf[256];
	FILE			*fp;
	long 			lret;
	//-----------------------------------
	// set menu item
	//-----------------------------------
	TermMenuSet(main_scr.menu[1]);
	//-----------------------------------
	// Menu process
	//-----------------------------------
	while(1){
		lret = TermGetMenuNum("Please input a menu number(0-8).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			printf("TermGetMenuNum RetCode = %ld\n", lret);
			return 0;
		}
		switch(menu_num){
		//--------------------------------------------
		// Initialization
		//--------------------------------------------
		case MENU_DIO_OPEN:
			lret = TermGetStr("Please input a device name.", get_str);
			if(lret != TERM_ERR_SUCCESS){
				printf("TermGetStr RetCode = %ld\n", lret);
				return 0;
			}
			lret = DioInit(get_str, &id_master);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioInit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Setting
		//--------------------------------------------
		case MENU_CONDITION:
			lret = master_condition(get_str);
			if( lret == 1){
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, get_str);
			}
			break;
		//--------------------------------------------
		// Start
		//--------------------------------------------
		case MENU_START:
			//----------------------------
			// Status clear
			//----------------------------
			TermSetStatus(STATUS_AREA_TOP, STS_COUNT_POS, "");
			TermSetStatus(STATUS_AREA_TOP, STS_STATUS_POS, "");
			TermSetStatus(STATUS_AREA_TOP, STS_ERROR_POS, "");
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, "");
			//----------------------------
			// Notification
			//----------------------------
			lret = DioDmSetStopEvent(id_master, dir,(PDIO_STOP_CALLBACK)stop_call_back_master, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetStopEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
				break;
			}
			notification = set_master_scr.input_num[SET_NUM_NOTIF].set_num;
			lret = DioDmSetCountEvent(id_master, dir, notification, (PDIO_COUNT_CALLBACK)count_call_back_master, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetCountEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Reset
			//----------------------------
			lret = DioDmReset(id_master, DIODM_RESET_FIFO_IN);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmReset %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Set Buffer
			//----------------------------
			lret = DioDmSetBuff(id_master, dir, data_buff_master, DATA_SIZE_MASTER, is_ring_master);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetBuff %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Start
			//----------------------------
			lret = DioDmStart(id_master, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStart %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Stop
		//--------------------------------------------
		case MENU_STOP:
			lret = DioDmStop(id_master, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStop %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Save data to file
		//--------------------------------------------
		case MENU_DATA:
			//----------------------------
			// Open file
			//----------------------------
			if ((fp = fopen(file_name_master, "w")) == NULL) {
				strcpy(buf, "fopen : file open error.");
				TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
				break;
			}

			//----------------------------
			// Write to file
			//----------------------------
			for (i=0; i<DATA_SIZE_MASTER; i++) {
				sprintf(buf, "%8ld : %08lX\n", i, data_buff_master[i]);
				fputs(buf, fp); 
			}
			//----------------------------
			// Close file
			//----------------------------
			fclose(fp);
			sprintf(buf, "Save data to file: %s", file_name_master);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Retrieve transfer count
		//--------------------------------------------
		case MENU_COUNT:
			lret = DioDmGetCount(id_master, dir, &count, &carry);
			if(lret == TERM_ERR_SUCCESS){
				sprintf(buf, "%lu", count);
				TermSetStatus(STATUS_AREA_TOP, STS_COUNT_POS, buf);
			}
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmGetCount %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Retrieve status
		//--------------------------------------------
		case NENU_STATUS:
			lret = DioDmGetStatus(id_master, dir, &status, &err);
			if(lret == TERM_ERR_SUCCESS){
				// Status
				strcpy(buf, "");
				if (status & DIODM_STATUS_BMSTOP)	strcpy(buf, "BmStop,");
				if (status & DIODM_STATUS_PIOSTART)	strcat(buf, "PioStart,");
				if (status & DIODM_STATUS_PIOSTOP)	strcat(buf, "PioStop,");
				if (status & DIODM_STATUS_TRGIN)	strcat(buf, "TrgIn,");
				if (status & DIODM_STATUS_OVERRUN)	strcat(buf, "Overrun");
				TermSetStatus(STATUS_AREA_TOP, STS_STATUS_POS, buf);
				// Error
				strcpy(buf, "");
				if (err & DIODM_STATUS_FIFOEMPTY)	strcpy(buf, "FifoEmpty,");
				if (err & DIODM_STATUS_FIFOFULL)	strcat(buf, "FifoFull,");
				if (err & DIODM_STATUS_SGOVERIN)	strcat(buf, "S/GOverIn,");
				if (err & DIODM_STATUS_TRGERR)		strcat(buf, "TrgErr,");
				if (err & DIODM_STATUS_CLKERR)		strcat(buf, "ClkErr,");
				if (err & DIODM_STATUS_SLAVEHALT)	strcat(buf, "SlaveHalt,");
				if (err & DIODM_STATUS_MASTERHALT)	strcat(buf, "MasterHalt");
				TermSetStatus(STATUS_AREA_TOP, STS_ERROR_POS, buf);
			}
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmGetStatus %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Termination
		//--------------------------------------------
		case MENU_DIO_CLOSE:
			lret = DioExit(id_master);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioExit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_TOP, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Close master screen
		//--------------------------------------------
		case MENU_RETURN:	
   			lret = TermMenuSet(main_scr.menu[0]);
			return 0;
			break;
		}
	}
	return 0;
}

//================================================================
//	Slave functon
//================================================================
int slave()
{
	int				menu_num;
	unsigned long	dir = DIODM_DIR_IN;
	unsigned long	i, count, carry, status, err, notification;
	char			get_str[256], buf[256];
	FILE			*fp;
	long 			lret;
	//-----------------------------------
	// Set menu item
	//-----------------------------------
	TermMenuSet(main_scr.menu[2]);
	//-----------------------------------
	// Menu process
	//-----------------------------------
	while(1){
		lret = TermGetMenuNum("Please input a menu number(0-8).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			printf("TermGetMenuNum RetCode = %ld\n", lret);
			return 0;
		}
		switch(menu_num){
		//--------------------------------------------
		// Initialization
		//--------------------------------------------
		case MENU_DIO_OPEN:
			lret = TermGetStr("Please input a device name.", get_str);
			if(lret != TERM_ERR_SUCCESS){
				printf("TermGetStr RetCode = %ld\n", lret);
				return 0;
			}
			lret = DioInit(get_str, &id_slave);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioInit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Setting
		//--------------------------------------------
		case MENU_CONDITION:
			lret = slave_condition(get_str);
			if( lret == 1){
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, get_str);
			}
			break;
		//--------------------------------------------
		// Start
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
			lret = DioDmSetStopEvent(id_slave, dir,(PDIO_STOP_CALLBACK)stop_call_back_slave, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetStopEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			notification = set_slave_scr.input_num[SET_SLAVE_NUM_NOTIF].set_num;
			lret = DioDmSetCountEvent(id_slave, dir, notification, (PDIO_COUNT_CALLBACK)count_call_back_slave, (void *)"");
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetCountEvent %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Reset
			//----------------------------
			lret = DioDmReset(id_slave, DIODM_RESET_FIFO_IN);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmReset %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Set Buffer
			//----------------------------
			lret = DioDmSetBuff(id_slave, dir, data_buff_slave, DATA_SIZE_SLAVE, is_ring_slave);
			if (lret != DIO_ERR_SUCCESS){
				DioGetErrorString(lret, get_str);
				sprintf(buf, "DioDmSetBuff %ld:%s", lret, get_str);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Start
			//----------------------------
			lret = DioDmStart(id_slave, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStart %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Stop
		//--------------------------------------------
		case MENU_STOP:
			lret = DioDmStop(id_slave, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStop %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Save data to file
		//--------------------------------------------
		case MENU_DATA:
			//----------------------------
			// Open file
			//----------------------------
			if ((fp = fopen(file_name_slave, "w")) == NULL) {
				strcpy(buf, "fopen : file open error.");
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
				break;
			}
			//----------------------------
			// Write to file
			//----------------------------
			for (i=0; i<DATA_SIZE_SLAVE; i++) {
				sprintf(buf, "%8ld : %08lX\n", i, data_buff_slave[i]);
				fputs(buf, fp);
			}
			//----------------------------
			// Close file
			//----------------------------
			fclose(fp);
			sprintf(buf, "Save data to file: %s", file_name_slave);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Retrieve transfer count
		//--------------------------------------------
		case MENU_COUNT:
			lret = DioDmGetCount(id_slave, dir, &count, &carry);
			if(lret == TERM_ERR_SUCCESS){
				sprintf(buf, "%lu", count);
				TermSetStatus(STATUS_AREA_BOTTOM, STS_COUNT_POS, buf);
			}

			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmGetCount %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Retrieve status
		//--------------------------------------------
		case NENU_STATUS:
			lret = DioDmGetStatus(id_slave, dir, &status, &err);
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
		// Termination
		//--------------------------------------------
		case MENU_DIO_CLOSE:
			lret = DioExit(id_slave);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioExit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Close slave screen
		//--------------------------------------------
		case MENU_RETURN:
			lret = TermMenuSet(main_scr.menu[0]);
			return 0;
			break;
		}
	}
	return 0;
}

//================================================================
//	Main function
//================================================================
int main()
{
	int				menu_num;
	long 			lret;
	//-----------------------------------
	// Initialize TermLib
	//-----------------------------------
	lret = TermInit(&main_scr);
	if(lret != TERM_ERR_SUCCESS){
		printf("TermInit RetCode = %ld\n", lret);
		return 0;
	}
	//-----------------------------------
	// Menu process
	//-----------------------------------
	while(1){
		lret = TermGetMenuNum("Please input a menu number(0-2).", &menu_num);
		if(lret != TERM_ERR_SUCCESS){
			printf("TermGetMenuNum RetCode = %ld\n", lret);
			return 0;
		}
		switch(menu_num){
		//--------------------------------------------
		// Master
		//--------------------------------------------
		case MENU_MASTER:
			master();
			break;
		//--------------------------------------------
		// Slave
		//--------------------------------------------
		case MENU_SLAVE:
			slave();
			break;
		//--------------------------------------------
		// Exit sample
		//--------------------------------------------
		case MENU_EXIT:
			TermExit();
			return 0;
			break;
		}
	}
	return 0;
}

