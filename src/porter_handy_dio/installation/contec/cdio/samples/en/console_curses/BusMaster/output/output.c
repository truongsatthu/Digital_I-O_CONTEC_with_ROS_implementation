//================================================================
//================================================================
//	CONTEC Linux DIO (BusMaster)
//	Output Sample
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
#define	MENU_DIO_OPEN		0
#define	MENU_CONDITION		1
#define	MENU_DATA_SET		2
#define	MENU_START			3
#define	MENU_STOP			4
#define	MENU_COUNT			5
#define	NENU_STATUS			6
#define	MENU_DIO_CLOSE		7
#define	MENU_EXIT			8
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
#define	SET_NUM_CLOCK		0
#define	SET_NUM_STOP_NUM	1
#define	SET_NUM_NOTIF		2

#define	DATA_SIZE			1000					// Data size
//================================================================
// External variable
//================================================================
static char				file_name[] = "output.txt";	// Name of saving file
static unsigned long	data_buff[DATA_SIZE];		// Data buffer
static unsigned long    is_ring;

//-----------------------------------
// Main screen data
//-----------------------------------
MAIN_SCR_DATA	main_scr = 
{
	"<< Output Sample >>",
	"",
	"",
	{
		{{"DioInit"},{"Condition..."},{"DataSet"},{"Start"},
		{"Stop"},{"Count"},{"Status"},{"DioExit"},{"Exit"}},
		{{""}}
	},
	{{""}},
	{{"Count"},{"Status"},{"Error"},{"Ret"},{""}}
};

//-----------------------------------
// Setting screen data
//-----------------------------------
SET_SCR_DATA	set_scr = 
{
	"<< Sampling Condition >>",
	{
		{{"Start"},{"Clock"},{"Stop"},
		{"Buffer"},{"Notification"},{"OK"},{"Cancel"},
		{""},{""},{""}},
		{{""}}
	},
	{	// Setting data for type of selected item
		{
			0,
			"Start",
			{
				{"Software Start",	DIODM_START_SOFT},
				{"External UP",		DIODM_START_EXT_RISE},
				{"External DOWN",	DIODM_START_EXT_FALL},
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
	{	//	Setting data for input numerical value
		{0, "Internal Clock",	"ns",	1000,	DEC_NUM},
		{1, "Stop Number",		"",		1000,	DEC_NUM},
		{2, "Number of Data",	"",		500,	DEC_NUM}
	}
};

//================================================================
//	CallBack function of transfer complete 
//================================================================
void stop_call_back(short id, short message, unsigned long dir, void *param)
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
// CallBack function of transfer complete in specified number 
//================================================================
void count_call_back(short id, short message, unsigned long dir, void *param)
{
	char get_str[256], buf[256];
	unsigned long count, carry;
	long lret;
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
// Initiarlize data file
//================================================================
long initial_data_file()
{
	FILE			*fp;
	unsigned long	i;
	char			buf[100], data_buf[256];
	//-----------------------------------
	// open file
	//-----------------------------------
	if ((fp = fopen(file_name, "w")) == NULL) {
		strcpy(buf, "fopen : file open error.");
		TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
		return 1;
	}
	//-----------------------------------
	// write file
	//-----------------------------------
	for (i=0; i<DATA_SIZE; i++) {
		sprintf(data_buf, "%8ld : %08lX\n", i, i);
		fputs(data_buf, fp); 
	}
	//-----------------------------------
	// close file
	//-----------------------------------
	fclose(fp);
	return 0;
}

//================================================================
//	Function for setting transfer condition
//================================================================
long condition(short id, char *err_str)
{
	char			get_str[256];
	int				menu_num;
	unsigned long	dir = DIODM_DIR_OUT; 
	unsigned long	start, clock, stop;
	unsigned long	internal_clock, stop_num, notification;
	unsigned long	def_set_num;
	long			lret;
	//-----------------------------------
	// Initialize setting data
	//-----------------------------------
	def_set_num		= set_scr.select_item[SET_MENU_START].set_num;
	start			= set_scr.select_item[SET_MENU_START].item[def_set_num].num;
	def_set_num		= set_scr.select_item[SET_MENU_CLOCK].set_num;
	clock			= set_scr.select_item[SET_MENU_CLOCK].item[def_set_num].num;
	def_set_num		= set_scr.select_item[SET_MENU_STOP].set_num;
	stop			= set_scr.select_item[SET_MENU_STOP].item[def_set_num].num;
	internal_clock	= set_scr.input_num[SET_NUM_CLOCK].set_num;
	stop_num		= set_scr.input_num[SET_NUM_STOP_NUM].set_num;
	notification	= set_scr.input_num[SET_NUM_NOTIF].set_num;
	//-----------------------------------
	// Initialize setting window
	//-----------------------------------
	lret = TermSetWindowOpen(&set_scr);
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
			lret = TermGetSelectItem(&set_scr, SET_MENU_START, &start);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set clock condition
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
		// Set stop condition
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
		//Buffer Condition
		//-----------------------------------
		case SET_MENU_BUF_CON:
			lret = TermGetSelectItem(&set_scr, SET_MENU_BUF_CON, &is_ring);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetSelectItem %ld\n", lret);
			}
			break;
		//-----------------------------------
		// Set notification count
		//-----------------------------------
		case SET_MENU_NOTIF:
			lret = TermGetInputNum(&set_scr, SET_NUM_NOTIF, &notification);
			if(lret != TERM_ERR_SUCCESS){
				sprintf(err_str, "TermGetInputNum %ld\n", lret);
			}
			break;
		//-----------------------------------
		// OK menu process
		//-----------------------------------
		case SET_MENU_OK:
			// Direction
			lret = DioDmSetDirection(id, PO_32);
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
//	Main function
//================================================================
int main()
{
	short			id;
	int				menu_num;
	unsigned long	dir = DIODM_DIR_OUT; 
	unsigned long	i, count, carry, status, err, notification;
	unsigned long	dummy;
	char			get_str[256], buf[256];
	long 			lret;
	FILE			*fp;
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
			lret = DioInit(get_str, &id);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioInit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Setting
		//--------------------------------------------
		case MENU_CONDITION:
			lret= condition(id, get_str);
			if( lret == 1){
				TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, get_str);
			}
			break;
		case MENU_DATA_SET:
			//-----------------------------------
			// Open file 
			//-----------------------------------
			if((fp = fopen(file_name, "r")) == NULL){
				//-----------------------------------
				// Since a data file did not open, data is created.
				//-----------------------------------
				if(initial_data_file() != 0){
					break;
				}
				//-----------------------------------
				// The created data file is opened.
				//-----------------------------------
				if ((fp = fopen(file_name, "r")) == NULL) {
					strcpy(buf, "fopen : file open error.");
					TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
					break;
				}
			}
			//-----------------------------------
			// Read data from file
			//-----------------------------------
			for(i = 0; i < DATA_SIZE; i++){
				fscanf(fp, "%8ld : %08lx\n", &dummy, &data_buff[i]);
			}
			//-----------------------------------
			// Close file
			//-----------------------------------
			fclose(fp);
			sprintf(buf, "Set data from file: %s", file_name);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
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
			lret = DioDmSetBuff(id, dir, data_buff, DATA_SIZE, is_ring);
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
		// Stop
		//--------------------------------------------
		case MENU_STOP:
			lret = DioDmStop(id, dir);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioDmStop %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
			break;
		//--------------------------------------------
		// Retrieve transfer count
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
		// Retrieve status
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
		// Termination
		//--------------------------------------------
		case MENU_DIO_CLOSE:
			lret = DioExit(id);
			DioGetErrorString(lret, get_str);
			sprintf(buf, "DioExit %ld:%s", lret, get_str);
			TermSetStatus(STATUS_AREA_BOTTOM, STS_RET_POS, buf);
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


