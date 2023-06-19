/*
    *     Copyright @ 2019-2020 Nguyen Truong 		- truongsatthu. All rights reserved.
*/

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int8.h" 
#include "dio84rd/dio_control.h"
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
double 	linear_vel  = 0; 
double 	angular_vel = 0;
int 	intputVal[8];
int 	outputVal[4];
int8_t 	outputCmd;

std::string dioDevName = "/dev/ttyUSB0";
int dioBaudRate = B115200;
dio_control::DioCommunication *dioCom;

void print_output(void)
{
    for (int i = 0; i < 4; i++)
    {
        printf("OUTPUT[%i]=%i\n", i, outputVal[i]);
    }
}

void reset_output_bit(void)
{
	printf("RESETTING BIT...\n");
	dioCom->writeOutput(0, outputVal);
	outputCmd = 0;
    //print_output();
	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");	
}

void set_output_bit(int bit, const int value)
{
	int8_t mask_bit = ~(1 << bit);
	if (!value){
		outputCmd = outputCmd & mask_bit;
	}
	else {
		outputCmd = outputCmd | (~mask_bit);
	} 
	printf("SETTING BIT...\n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++\n");
}

void get_vel(const geometry_msgs::Twist::ConstPtr& vel_msg)
{
	linear_vel  = vel_msg->linear.x;
	angular_vel = vel_msg->angular.z;
}

int main(int argc, char** argv)
{
    int 			OFF;
    int 			command;
    short 			output_port;
    static short 	output_old = -1;

    ros::init(argc, argv, "dio_driver");
    ros::NodeHandle n;
    ros::Subscriber sub_cmd_vel = n.subscribe("cmd_vel", 1000, get_vel);    
    
    //ROS_INFO_STREAM("Start dio_control");
    n.param("dioDevName", dioDevName, dioDevName);
    n.param("dioBaudRate", dioBaudRate, dioBaudRate);
    dioCom = new dio_control::DioCommunication;
    dioCom->init(dioDevName.c_str(), dioBaudRate);
    //ROS_INFO_STREAM("Initialize dio_control finished");
	
    ros::Rate freq(1);
    while(ros::ok())
{        	
        printf("linear vel  =%f\n", linear_vel);
        printf("angular vel =%f\n", angular_vel);

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
            case COMMAND_FORWARD:
		//find_output_port(output_port);
		//if (output_old != output_old){		
                reset_output_bit();
                set_output_bit(0, 1);
                dioCom->writeOutput(outputCmd, outputVal);
                //print_output();
                printf("---------->Going forward...\n");	
                break;
            case COMMAND_BACKWARD:
		//find_output_port(output_port);
		//if (output_old != output_old){
                reset_output_bit();
                set_output_bit(1, 1);
                dioCom->writeOutput(outputCmd, outputVal);
                //print_output();
                printf("---------->Going backward...\n");
                break;    
            case COMMAND_TURNLEFT:
		//find_output_port(output_port);
		//if (output_old != output_old){                
		reset_output_bit();
                set_output_bit(2, 1);
                dioCom->writeOutput(outputCmd, outputVal);
                //print_output();
                printf("---------->Turning left...\n");
                break;
            case COMMAND_TURNRIGHT:
		//find_output_port(output_port);
		//if (output_old != output_old){
                reset_output_bit();
                set_output_bit(3, 1);
                dioCom->writeOutput(outputCmd, outputVal);
                //print_output();
                printf("---------->Turning right...\n");
                break;
            }
		output_old = output_port;

            freq.sleep();
            ros::spinOnce();
    }

    return 0;
}
