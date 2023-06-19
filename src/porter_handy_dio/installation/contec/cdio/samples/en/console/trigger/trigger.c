//================================================================
//================================================================
// CONTEC Linux DIO
// Trigger Sample
//               				    				CONTEC Co.,Ltd.
//												Ver1.00
//================================================================
//================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/cdio.h"

//================================================================
// 	Call Back Function
//================================================================
void trg_call_back(short id, short msg, long bit_no, long logic, void *param)
{
	char	up_down[100];
	//------------------------------------------
	// 	Process Trigger Message
	//------------------------------------------
	if (msg == DIOM_TRIGGER) {
		if (logic == DIO_TRG_RISE) {
			strcpy(up_down, "UP");
		} else if (logic == DIO_TRG_FALL) {
			strcpy(up_down, "DOWN");
		} else {
			strcpy(up_down, "NONE");
		}
		fprintf(stdout, "Trigger:id=%d, bit = %d, logic = %s: %s\n", (int)id, (int)bit_no, up_down, (char *)param);
	}
	//------------------------------------------
	// 	The Others
	//------------------------------------------
	else {
		fprintf(stdout, "msg = %d\n", (int)msg);
	}
}

//================================================================
// 	Main Function
//================================================================
int	main(int argc, char *argv[])
{
	short			bit_no;
	char			buf[256];
	long			lret;
	char			device_name[50];
	short			id;
	short			max_trg = 0;
	short			in_port_num;
	short			out_port_num;
	int			c;
	//-------------------------------------
	// 	Command Line Interpretation
	//-------------------------------------
	if (argc == 2) {
		strcpy(device_name, argv[1]);
	} else if (argc == 3) {
		strcpy(device_name, argv[1]);
		max_trg = (short)atoi(argv[2]);
	} else {
		strcpy(device_name, "DIO000");

	}
	//-------------------------------------
	// 	Initializtion
	//-------------------------------------
	lret = DioInit(device_name, &id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioInit", lret, buf);
		return -1;
	}
	//-------------------------------------
	// 	Retrieve Number of Bits
	//-------------------------------------
	if (max_trg == 0) {
		in_port_num = 0;
		out_port_num = 0;
		lret = DioGetMaxPorts(id, &in_port_num, &out_port_num);
		if (lret != DIO_ERR_SUCCESS) {
			DioGetErrorString(lret, buf);
			fprintf(stderr, "%s=%ld:%s\n", "DioGetMaxPorts", lret, buf);
			return -1;
		}
		max_trg = in_port_num * 8;
	}
	if (max_trg == 0) {
		fprintf(stderr, "Input bit number = 0\n");
		return -1;
	} else {
		fprintf(stdout, "Trigger bit = 0 to %d.\n", (int)max_trg - 1);
	}
	//-------------------------------------
	// 	Set Trigger Call Back
	//-------------------------------------
	lret = DioSetTrgCallBackProc(id, (PDIO_TRG_CALLBACK)trg_call_back, (void *)"input 'q' + 'enter` to quit");
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioSetTrgCallBackProc", lret, buf);
		return -1;
	}
	//-------------------------------------
	// 	Trigger is Valid
	//-------------------------------------
	for (bit_no=0; bit_no<max_trg; bit_no++) {
		lret = DioSetTrgEvent(id, bit_no, (DIO_TRG_RISE | DIO_TRG_FALL), 100);
		if (lret != DIO_ERR_SUCCESS) {
			DioGetErrorString(lret, buf);
			fprintf(stderr, "%s=%ld:bit_no=%d:%s\n", "DioSetTrgEvent", lret, (int)bit_no, buf);
			return -1;
		}
	}
	fprintf(stdout, "Waiting for trigger...\n");
	//-------------------------------------
	// 	Wait Input
	//-------------------------------------
	while (1) {
		fprintf(stdout, "input 'q' + 'enter' to quit\n");
		c = fgetc(stdin);
		if (c == 'q') {
			break;
		}
	}
	//-------------------------------------
	// 	Exit
	//-------------------------------------
	lret = DioExit(id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioExit", lret, buf);
		return -1;
	}
	return 0;
}


