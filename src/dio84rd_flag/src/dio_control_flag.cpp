/*
    *     Copyright @ 2019-2020 Nguyen Truong 		- truongsatthu. All rights reserved.
*/

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int8.h" 
#include "dio84rd_flag/dio_control_flag.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <bits/stdc++.h> 
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

std::string dioDevName = "/dev/ttyUSB0";
int dioBaudRate = B115200;
dio_control::DioCommunication *dioCom;

int inputVal[8];
int outputVal[4];

void print_input(void)
{
    for (int i = 0; i < 8; i++)
    {
        printf("OUTPUT[%i]=%i\n", i, inputVal[i]);
    }
}

void make_warnings(short port_no)
{
	int warnings	= 0;
	for(int i =0; i < 8; i++){
	if (inputVal[i] != 0) {
		if (port_no == 2 or port_no == 5){
		printf("WARNING_port=%d,\t input_bit = %02X\n", port_no, inputVal[port_no]);
		warnings = 1;
		}
	} else {
		warnings = 0;
			}
		}
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");	
}

void make_alerts(short port_no)
{
	int alerts	= 0;
	for(int i =0; i < 8; i++){
	if (inputVal[i] != 0) {
		if (port_no == 3 or port_no == 6){
		printf("ALERT_port=%d,\t input_bit = %02X\n", port_no, inputVal[port_no]);
		alerts = 1;
		}
	} else {
		alerts = 0;
			}
		}
	printf("*********************************************\n");	
}

int main(int argc, char** argv)
{
    short 		port_no;
    int			warnings, alerts;

    ros::init(argc, argv, "dio_driver");
    ros::NodeHandle n;  
    
    //ROS_INFO_STREAM("Start dio_control");
    n.param("dioDevName", dioDevName, dioDevName);
    n.param("dioBaudRate", dioBaudRate, dioBaudRate);
    dioCom = new dio_control::DioCommunication;
    dioCom->init(dioDevName.c_str(), dioBaudRate);
    dioCom->readInput(inputVal);
    //ROS_INFO_STREAM("Initialize dio_control finished");

    ros::Rate freq(1);
    while(ros::ok())
	{       
		print_input();
		make_warnings(2);
		make_warnings(5);
		make_alerts	(3);
		make_alerts(6);
        printf("warning[%i]\t=%i\n", port_no, warnings);
        printf("alert  [%i]\t=%i\n", port_no, alerts);
		ros::Publisher dio_pub1 = n.advertise<std_msgs::Int8>("dio_warnings", 1000);
		std_msgs::Int8 flags_warnings;
		flags_warnings.data 	= warnings;
		dio_pub1.publish(flags_warnings);

		ros::Publisher dio_pub2 = n.advertise<std_msgs::Int8>("dio_alerts", 1000);
		std_msgs::Int8 flags_alerts;
		flags_alerts.data  	= alerts;
		dio_pub2.publish(flags_alerts);

		freq.sleep();
		ros::spinOnce();
    }

    return 0;
}
