#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int8.h" 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdio.h"
using namespace std;
////////////////////////////////////////////
// For TF & Odom			////
#include <geometry_msgs/Twist.h>	////
#include <nav_msgs/Odometry.h>		////
#include <tf/transform_broadcaster.h>	////
////////////////////////////////////////////
//================================================================
// 	Command Define
//================================================================
#define	COMMAND_ERROR		0	// Error
#define	COMMAND_PREP		1	// 1
#define	COMMAND_FORWARD		2	// 1 
#define	COMMAND_BACKWARD	3	// 1 
#define	COMMAND_TURNLEFT	4	// 1 
#define	COMMAND_TURNRIGHT	5	// 1  

//-----------+++global variables+++-----------------------------//
	double 			linear_vel  = 0; 
	double 			angular_vel = 0;
	
void make_warnings(short port_no,short id,long lret,char buf[256],unsigned char &data)
{
	int warnings	= 0;
	lret = DioInpBit(id, port_no, &data);
	if (lret == DIO_ERR_SUCCESS ) {
		if (port_no == 2 or port_no == 5){
		fprintf(stdout, "WARNING_port=%d,\t input_data = %02X\n", (int)port_no, (int)data);
		warnings = 1;
		}
	} else {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioInpBit", lret, buf);
		warnings = 0;
			}
	fprintf(stdout, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");	
}

void make_alerts(short port_no,short id,long lret,char buf[256],unsigned char &data)
{
	int alerts	= 0;
	lret = DioInpBit(id, port_no, &data);
	if (lret == DIO_ERR_SUCCESS) {
		if (port_no == 3 or port_no == 6){
		fprintf(stdout, "ALERT_port=%d,\t input_data = %02X\n", (int)port_no, (int)data);
		alerts = 1;
		}
	} else {
		DioGetErrorString(lret, buf);
		fprintf(stderr, "%s=%ld:%s\n", "DioInpBit", lret, buf);
		alerts = 0;
			}
	fprintf(stdout, "*********************************************\n");	
}

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

void get_vel(const geometry_msgs::Twist::ConstPtr& vel_msg)
{
	linear_vel  = vel_msg->linear.x;
	angular_vel = vel_msg->angular.z;
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "dio");
	ros::NodeHandle n;	
	ros::Subscriber sub_cmd_vel = n.subscribe("cmd_vel", 1000, get_vel);

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
	static short 		port_no_old	=-1;
	static unsigned char 	data_old	=-1;
	int			warnings, alerts;
	
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

ros::Rate freq(10);
while(ros::ok())
{

make_warnings(2, id, lret, buf, data);
make_warnings(5, id, lret, buf, data);
make_alerts(3, id, lret, buf, data);
make_alerts(6, id, lret, buf, data);

  ros::Publisher dio_pub1 = n.advertise<std_msgs::Int8>("dio_warnings", 1000);
  std_msgs::Int8 flags_warnings;
  flags_warnings.data 	= warnings;
  dio_pub1.publish(flags_warnings);

  ros::Publisher dio_pub2 = n.advertise<std_msgs::Int8>("dio_alerts", 1000);
  std_msgs::Int8 flags_alerts;
  flags_alerts.data  	= alerts;
  dio_pub2.publish(flags_alerts);
  
if (linear_vel==0 && angular_vel==0){
	command = COMMAND_PREP;
}

if (linear_vel>0 && angular_vel==0){
	command = COMMAND_FORWARD;
}
	
if (linear_vel<0 && angular_vel==0){
	command = COMMAND_BACKWARD;
}
	
if (angular_vel<0){
	command = COMMAND_TURNLEFT;
}

if (angular_vel>0){
	command = COMMAND_TURNRIGHT;
}

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
				fprintf(stdout, "---------->Preparing!\n");				
			break;
		case COMMAND_FORWARD:		// 1 Port Output
			if (port_no != port_no_old&& port_no_old!=-1){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);	
				fprintf(stdout, "---------->Going forward...\n");	
			break;
		case COMMAND_BACKWARD:		// 1 Port Output
			if (port_no != port_no_old&& port_no_old!=-1){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
				fprintf(stdout, "---------->Going backward...\n");
			break;
		case COMMAND_TURNLEFT:		// 1 Port Output
			if (port_no != port_no_old&& port_no_old!=-1){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
				fprintf(stdout, "---------->Turning left...\n");
			break;
		case COMMAND_TURNRIGHT:		// 1 Port Output
			if (port_no != port_no_old&& port_no_old!=-1){
				reset_port_data(port_no_old,id,lret,buf);
			}
			set_port_data(port_no,id,lret,buf);
				fprintf(stdout, "---------->Turning right...\n");
			break;
		}
	
		port_no_old 	=port_no;
		data_old 	=data;

freq.sleep();
ros::spinOnce();

}		//end while	
  return 0;
}		//end main

/***************************************************************
Extra codes to subcribe odometry instead of subcribing cmd_vel
Consider to replace(void)/subcribe: "get_odom==get_vel" 
***************************************************************/
/*
void get_odom(const nav_msgs::Odometry::ConstPtr& msg)
{
	linear_vel  = msg->twist.twist.linear.x;
	angular_vel = msg->twist.twist.angular.z;
}
	ros::Subscriber sub = n.subscribe("odom", 1000, get_odom);
*/
