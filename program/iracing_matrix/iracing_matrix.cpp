/*
# Copyright (c) 2023 Jelle Hennevelt <jelle_AT_hennevelt_DOT_nl>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# SPDX-License-Identifier: BSD-3-Clause
*/

// Minimal winver Windows XP
#define MIN_WIN_VER 0x0501

#ifndef WINVER
#	define WINVER			MIN_WIN_VER
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT		MIN_WIN_VER 
#endif

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <conio.h>
#include <signal.h>
#include <string>

#include "irsdk_defines.h"
#include "irsdk_client.h"
#include "yaml_parser.h"
#include "irsdk\irsdk_ir2ad\serial.h"

// for timeBeginPeriod
#pragma comment(lib, "Winmm")


//PitRoad: OnPitRoad == 1 and IsOnTrack == 1
//PitBox: IsInGarage == 0 and IsOnTrack == 0
irsdkCVar g_playerInGarage("IsInGarage");
irsdkCVar g_playerOnPitRoad("OnPitRoad");
irsdkCVar g_playerInCar("IsOnTrack");
irsdkCVar g_pitMaxSpeed("TrackPitSpeedLimit");
irsdkCVar g_carSpeed("Speed");
irsdkCVar g_carRPM("RPM");
irsdkCVar g_carGear("Gear");
irsdkCVar g_carBrake("Brake");



Serial serial;

int comPORT = -1;
bool firstConnect = true;
char* exeName = "iracing_matrix.exe";
char rxDetect[15] = "iracing_matrix";


void help()
{
	printf("iRacing Matrix usage\n\n%s [port]\n\nport\tEnter COM port number\n", exeName);
	printf("\tLeave empty for auto detect\n\nNote that you can use ctrl-c to exit\n");
	exit(0);
}

void stop(int reset=0)
{
	serial.writeSerialPrintf("!E%i\n", reset);
}

void ex_program(int sig) 
{
	(void)sig;

	printf("\n\nReceived ctrl-c, exiting\n");
	serial.closeSerial();
	timeEndPeriod(1);

	signal(SIGINT, SIG_DFL);
	exit(0);
}

bool init()
{
	// trap ctrl-c
	signal(SIGINT, ex_program);
	printf("[ctrl-c]\n\n");

	// bump priority up so we get time from the sim
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	// ask for 1ms timer so sleeps are more precise
	timeBeginPeriod(1);


	int port;
	if (comPORT == -1) {
		
		int portList[32];
		int portCount = 32;
		port = serial.enumeratePorts(portList, &portCount);

		printf("Detected COM ports\t");
		for (int i = 0; i < portCount; i++) {
			printf("COM%i ", portList[i]);
		}
		printf("\n\n");

		// Try all detected com ports
		for (int i = 0; i < portCount; i++) {
			port = portList[i];
			printf("Connecting to COM%i\t", port);

			// open serial
			if (serial.openSerial(port, CBR_115200))
			{
				printf("Connected\nDetecting hardware\t");

				bool matrix_gearFound = false;
				uint8_t detectCount = 0;
				while (!matrix_gearFound && detectCount < 30) {
					Sleep(100);
					detectCount++;
					char rxMSG[sizeof(rxDetect)] = "";
					DWORD rxLen = serial.readSerial(rxMSG, sizeof(rxDetect));
					if (rxLen > 1) {
						if (strcmp(rxMSG, rxDetect) == 0) {
							matrix_gearFound = true;
						}
					}
				}
				if (matrix_gearFound) {
					printf("%s\n", rxDetect);
					return true;
				}
				else {
					printf("Timeout\n");
					//return false;
				}
			}
			else {
				printf("Failed\n");
			}
		}
		printf("\nAuto detect failed\tDefine port manualy, -h for help\n");
		return false;
	}
	else {
		port = comPORT;
		printf("Connecting to COM%i\t", port);
		// open serial
		if (serial.openSerial(port, CBR_115200))
		{
			printf("Connected\n");
			return true;
		}
		else {
			printf("Failed\n");
			return false;
		}

	}
}



void monitorConnectionStatus()
{
	// keep track of connection status
	if (firstConnect)
		printf("\nConnecting iRacing\t");

	bool isConnected = irsdkClient::instance().isConnected();
	static bool wasConnected = !isConnected;
	if(wasConnected != isConnected)
	{
		if (isConnected) {
			if (firstConnect){
				printf("Connected");
			}
			else {
				printf("\b\b\b\b\b\b\b\b\bConnected");
			}
		}
		else {
			if (firstConnect) {
				printf("Waiting  ");
			}
			else {
				printf("\b\b\b\b\b\b\b\b\bWaiting  ");
			}
			stop(1);
		}
		firstConnect = false;
		wasConnected = isConnected;
	}
}



void run()
{
	// wait up to 16 ms for start of session or new data
	if(irsdkClient::instance().waitForData(16))
	{
		// and grab the data
		if(g_playerInCar.getBool())
		{
			float BRAKE = g_carBrake.getFloat();
			int GEAR = g_carGear.getInt();
			int RPM = g_carRPM.getInt();
			float SPEED = g_carSpeed.getFloat();

			// Brake
			serial.writeSerialPrintf("!B%f\n", BRAKE);
			Sleep(20);

			// Gear
			serial.writeSerialPrintf("!G%i\n", GEAR);
			Sleep(20);

			// RPM
			serial.writeSerialPrintf("!R%i\n", RPM);
			Sleep(20);

			// SPEED
			serial.writeSerialPrintf("!S%f\n", SPEED);
			Sleep(20);
		}
		else {
			stop();
		}
	}
	serial.writeSerialPrintf("!C\n");
	Sleep(20);

	// normal process loop
	monitorConnectionStatus();
}

int main(int argc, char *argv[])
{
	exeName = argv[0];
	if (argc > 1)
	{
		if ((strcmp(argv[1],"-h") == 0) || (strcmp(argv[1], "/h") == 0) || (strcmp(argv[1], "/?") == 0)) {
			help();
			exit(0);
		}
		else if ((comPORT = strtol(argv[1], NULL, 10)) == false) {
			help();
			printf("\nError: port should be a number\n");
			exit(0);
		}
	}
	
	printf("iRacing Matrix\n\n");
	printf("Exit keybinding\t\t");

	if (init())
	{
		while (true)
		{
			run();
		}
		stop();
		printf("\n\nShutting down.\n");
		serial.closeSerial();
		timeEndPeriod(1);
	}
	else
		// quit
		exit(1);

	return 0;
}
