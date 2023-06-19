/*
    *     Copyright @ 2019-2020 Nguyen Truong 		- truongsatthu. All rights reserved.
    launch script for dio control
 */

#ifndef __DIO_CONTROL_H__
#define __DIO_CONTROL_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <iostream>

namespace dio_control
{
    class DioCommunication{
        private:
            int fd_;
            struct termios old_stdio;
            struct termios stdio;
            struct termios tio;
            bool debugFlag_;

            int res_, txs_, recpoint_;
            long rxtout_;
            char buf_[256], txtbuf_[256];
            char recIntput_[256];
            char recOutput_[256];
            char c;

            void sendCommand(char *cmd, char *rec)
            {
                sprintf(txtbuf_, "%s", cmd);
                txs_ = strlen(txtbuf_);
                //        if (debugFlag_)
                //            printf("TX data > %s\n", txtbuf_);
                txtbuf_[txs_ - 1] = 0xa;
                res_ = read(fd_, buf_, 40);
                write(fd_, txtbuf_, txs_);
                recpoint_ = rxtout_ = 0;
                do
                {
                    res_ = read(fd_, buf_, 40);
                    if (res_ != 0)
                    {
                        for (int i = 0; i != res_; i++)
                        {
                            rec[recpoint_] = buf_[i];
                            recpoint_++;
                            if (recpoint_ > 200)
                                break;
                        }
                    }
                    rec[recpoint_] = 0;
                    rxtout_ = 0;
                    res_--;
                    if ((buf_[res_] == 0xa))
                        break;
                    else
                        rxtout_++;
                } while (rxtout_ < 2000000);

                //        if (debugFlag_ && recpoint_ != 0)
                //            printf("RX data > %s",rec);
            }

        public:
            DioCommunication(){

            };

            ~DioCommunication() {};

            void init(const char *devName, int baudRate){
                if ((fd_ = open(devName, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)
                {
                    perror(devName);
                    exit(1);
                }

                tcgetattr(STDOUT_FILENO, &old_stdio);
                bzero(&tio, sizeof(tio));                
                tio.c_cflag = (baudRate | CS8 | CLOCAL | CREAD);
                tio.c_iflag = 0;
                tio.c_oflag = 0;
                tio.c_lflag = 0;
                tcflush(fd_, TCIFLUSH);
                tcsetattr(fd_, TCSANOW, &tio);
            };

            void term(void){
                tcsetattr(fd_, TCSANOW, &old_stdio);
                close(fd_);
            };

            void readInput(int* returnValue){
                //printf("Read input...");
                sendCommand((char *)"Input-", recIntput_);
                // Interpre feedback and implement code here
                if (strlen(recIntput_)<5)
                    returnValue[0] = -1;
                else {
                    int recLen = strlen(recIntput_);
                    for (int i = 0; i < 8; i++)
                    {
                        //printf("%c", recIntput_[i+6]);
                        if (recIntput_[i+6]=='1')
                            returnValue[i] = 1;
                        else
                            returnValue[i] = 0;                        
                    }
                }
            };

            void writeOutput(int8_t outValue, int* returnValue){
                char tx_buf[256];
                //printf("Write output...");
                sprintf(tx_buf, "Output%i-", outValue);
                sendCommand(tx_buf, recOutput_);
                // Interpre feedback and implement code here
                if (strlen(recIntput_) < 5)
                    returnValue[0] = -1;
                else
                {
                    int recLen = strlen(recOutput_);
                    for (int i = 0; i < 4; i++)
                    {
                        //printf("%c", recOutput_[i+7]);
                        if (recOutput_[i + 7]=='1')
                            returnValue[i] = 1;
                        else
                            returnValue[i] = 0;
                    }
                }
            };
    };   // class DioCommunication
} // namespace dio_control

#endif
