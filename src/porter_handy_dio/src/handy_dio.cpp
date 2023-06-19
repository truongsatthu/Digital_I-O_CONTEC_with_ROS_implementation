#include "ros/ros.h"
#include "std_msgs/String.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdio.h"

using namespace std;

//================================================================
// 	Command Define
//================================================================
#define	COMMAND_ERROR		0	// Error
#define	COMMAND_PREP		1	// 1
#define	COMMAND_FORWARD		2	// 1 
#define	COMMAND_BACKWARD	3	// 1 
#define	COMMAND_TURNLEFT	4	// 1 
#define	COMMAND_TURNRIGHT	5	// 1  
#define	COMMAND_QUIT		6	// End

void reset_port_data(short port_no,short id, long lret, char buf[256])
{
	int data;
	data = 1;
	lret = DioOutBit(id, port_no, data);
	if (lret == DIO_ERR_SUCCESS) {
		fprintf(stdout, "RESET 'Port no. %d' TO 'data = %02X'\n", (int)port_no, (int)data);
	} else {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "RESET FAILED!!! %s=%ld:%s\n", "DioOutBit", lret, buf);
	}
	fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++\n");	
}
void set_port_data(short port_no,short id,long lret,char buf[256])
{
	int data;
	data = 0;
	lret = DioOutBit(id, port_no, data);
	if (lret == DIO_ERR_SUCCESS) {
		fprintf(stdout, "SET 'Port no. %d', data = %02X\n", (int)port_no, (int)data);
	} else {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioOutBit", lret, buf);
	}	
	fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++\n");
}

int main(int argc, char **argv)
{
//================================================================
// 	DIO device initialization
//================================================================
	short			port_no;
	short			bit_no;
	unsigned char  		data;
	char			buf[256];
	int			command;
	long			lret;
	char			device_name[50];
	short			id;
	int			itmp;
	static short 		port_no_old=-1;
	static unsigned char 	data_old=-1;
	
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
//================================================================

while (1)
  {

		//-------------------------------------
		//		Display Command
		//-------------------------------------
		fprintf(stdout, "P   : Preparation\n");
		fprintf(stdout, "W   : Move forward\n");
		fprintf(stdout, "S   : Move backward\n");
		fprintf(stdout, "A   : Turn left\n");
		fprintf(stdout, "D   : Turn right\n");
		fprintf(stdout, "q   : quit\n");
		fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++\n");
		fprintf(stdout, "input command:");
		//-------------------------------------
		// 		Wait Input
		//-------------------------------------

		fgets(buf, sizeof(buf), stdin);

		//-------------------------------------
		// 		Distinguish Command
		//-------------------------------------
		command = COMMAND_ERROR;

		if (strstr(buf, "p") != NULL) {
			command = COMMAND_PREP;			// 1 Port Input
			fprintf(stdout, "Preparation signal received\n");
		}
		if (strstr(buf, "w") != NULL) {
			command = COMMAND_FORWARD;		// 1 Port Input
			fprintf(stdout, "Forward signal received\n");
		}
		if (strstr(buf, "s") != NULL) {
			command = COMMAND_BACKWARD;		// 1 Port Input
			fprintf(stdout, "Backward signal received\n");
		}
		if (strstr(buf, "a") != NULL) {
			command = COMMAND_TURNLEFT;		// 1 Port Input
			fprintf(stdout, "Turn left signal received\n");
		}
		if (strstr(buf, "d") != NULL) {
			command = COMMAND_TURNRIGHT;		// 1 Port Input
			fprintf(stdout, "Turn right signal received\n");
		}
		if (strstr(buf, "q") != NULL) {
			command = COMMAND_QUIT;			// End
		}

		//-------------------------------------
		// 		Choosing commander
		//-------------------------------------
		switch (command) {
		case COMMAND_PREP:
			port_no = 0;
			break;
		case COMMAND_FORWARD:
			port_no = 1;
			break;
		case COMMAND_BACKWARD:
			port_no = 2;
			break;	
		case COMMAND_TURNLEFT:
			port_no = 3;
			break;
		case COMMAND_TURNRIGHT:
			port_no = 4;
			break;
		default:
			break;
		}

		//-------------------------------------
		// Execute Command and Display Result
		//-------------------------------------
		switch (command) {
		case COMMAND_ERROR:		// Error
			fprintf(stderr, "error:%s", buf);
			fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++\n");
			break;
		case COMMAND_PREP:		// 1 Port Output
			if (port_no != port_no_old && port_no_old!=-1){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);				
			break;
		case COMMAND_FORWARD:		// 1 Port Output
			if (port_no != port_no_old){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);		
			break;
		case COMMAND_BACKWARD:		// 1 Port Output
			if (port_no != port_no_old){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
			break;
		case COMMAND_TURNLEFT:		// 1 Port Output
			if (port_no != port_no_old){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
			break;
		case COMMAND_TURNRIGHT:		// 1 Port Output
			if (port_no != port_no_old){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
			break;
		case COMMAND_QUIT:		// End
			fprintf(stdout, "quit.\n");
			goto END;
			break;	
		}

		port_no_old 	=port_no;
		data_old 	=data; 

  }	//end while

END:
	lret = DioExit(id);
	if (lret != DIO_ERR_SUCCESS) {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioExit", lret, buf);
		return -1;
	}
  return 0;
}	//end main
