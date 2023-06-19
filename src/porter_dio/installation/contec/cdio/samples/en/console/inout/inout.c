//================================================================
//================================================================
// CONTEC Linux DIO
// Input and Output Sample
//               				    				CONTEC Co.,Ltd.
//												Ver1.00
//================================================================
//================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../inc/cdio.h"

//================================================================
// 	Command Define
//================================================================
#define	COMMAND_ERROR		0	// Error
#define	COMMAND_INP_PORT	1	// 1 Port Input
#define	COMMAND_INP_BIT		2	// 1 Bit Input
#define	COMMAND_OUT_PORT	3	// 1 Port Output
#define	COMMAND_OUT_BIT		4	// 1 Bit Output
#define	COMMAND_ECHO_PORT	5	// 1 Port Echo Back
#define	COMMAND_ECHO_BIT	6	// 1 Bit Echo Back
#define	COMMAND_QUIT		7	// End

//================================================================
// 	Main Function
//================================================================
int	main(int argc, char *argv[])
{
	short			port_no;
	short			bit_no;
	unsigned char  	data;
	char			buf[256];
	int			command;
	long			lret;
	char			device_name[50];
	short			id;
	int			itmp;
	//-------------------------------------
	// 	Command Line Interpretation
	//-------------------------------------
	if (argc == 2) {
		strcpy(device_name, argv[1]);
	} else {
		strcpy(device_name, "DIO000");
	}
	//-------------------------------------
	// 	Initialization
	//-------------------------------------
	lret = DioInit(device_name, &id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioInit", lret, buf);
		return -1;
	}
	//-------------------------------------
	// 	Loop that Wait Input
	//-------------------------------------
	while(1) {
		//-------------------------------------
		//		Display Command
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
		// 		Wait Input
		//-------------------------------------
		fgets(buf, sizeof(buf), stdin);
		//-------------------------------------
		// 		Distinguish Command
		//-------------------------------------
		command = COMMAND_ERROR;
		if (strstr(buf, "ip") != NULL) {
			command = COMMAND_INP_PORT;		// 1 Port Input
		}
		if (strstr(buf, "ib") != NULL) {
			command = COMMAND_INP_BIT;		// 1 Bit Input
		}
		if (strstr(buf, "op") != NULL) {
			command = COMMAND_OUT_PORT;		// 1 Port Output
		}
		if (strstr(buf, "ob") != NULL) {
			command = COMMAND_OUT_BIT;		// 1 Bit Output
		}
		if (strstr(buf, "ep") != NULL) {
			command = COMMAND_ECHO_PORT;    // 1 Port Echo Back
		}
		if (strstr(buf, "eb") != NULL) {
			command = COMMAND_ECHO_BIT;		// 1 Bit Echo Back
		}
		if (strstr(buf, "q") != NULL) {
			command = COMMAND_QUIT;			// End
		}
		//-------------------------------------
		// 		Input Port Number and Bit Number
		//-------------------------------------
		switch (command) {
		case COMMAND_INP_PORT:		// 1 Port Input
		case COMMAND_OUT_PORT:		// 1 Port Output
		case COMMAND_ECHO_PORT:		// 1 Port Echo Back
			fprintf(stdout, "input port number:");
			fgets(buf, sizeof(buf), stdin);
			port_no = (short)atoi(buf);
			break;
		case COMMAND_INP_BIT:		// 1 Bit Input
		case COMMAND_OUT_BIT:		// 1 Bit Output
		case COMMAND_ECHO_BIT:		// 1 Bit Echo Back
			fprintf(stdout, "input bit number:");
			fgets(buf, sizeof(buf), stdin);
			bit_no = (short)atoi(buf);
			break;
		default:
			break;
		}
		//-------------------------------------
		// 		Input Data
		//-------------------------------------
		switch (command) {
		case COMMAND_OUT_PORT:		// 1 Port Output
		case COMMAND_OUT_BIT:		// 1 Bit Output
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
		// 		Execute Command and Display Result
		//-------------------------------------
		switch (command) {
		case COMMAND_INP_PORT:		// 1 Port Input
			lret = DioInpByte(id, port_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioInpByte", lret, buf);
			}
			break;
		case COMMAND_INP_BIT:		// 1 Bit Input
			lret = DioInpBit(id, bit_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioInpBit", lret, buf);
			}
			break;
		case COMMAND_OUT_PORT:		// 1 Port Output
			lret = DioOutByte(id, port_no, data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioOutByte", lret, buf);
			}
			break;
		case COMMAND_OUT_BIT:		// 1 Bit Output
			lret = DioOutBit(id, bit_no, data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioOutBit", lret, buf);
			}
			break;
		case COMMAND_ECHO_PORT:		// 1 Port Echo Back
			lret = DioEchoBackByte(id, port_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "port=%d, data = %02X\n", (int)port_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioEchoBackByte", lret, buf);
			}
			break;
		case COMMAND_ECHO_BIT:		// 1 Bit Echo Back
			lret = DioEchoBackBit(id, bit_no, &data);
			if (lret == DIO_ERR_SUCCESS) {
				fprintf(stdout, "bit=%d, data = %02X\n", (int)bit_no, (int)data);
			} else {
				DioGetErrorString(lret, buf);
				fprintf(stderr, "%s=%ld:%s\n", "DioEchoBackBit", lret, buf);
			}
			break;
		case COMMAND_QUIT:			// End
			fprintf(stdout, "quit.\n");
			goto END;
			break;
		case COMMAND_ERROR:			// Error
			fprintf(stderr, "error:%s", buf);
			break;
		}
	}
	//-------------------------------------
	// 	Exit
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


