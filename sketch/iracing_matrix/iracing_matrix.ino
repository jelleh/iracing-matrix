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

#include "LedControl.h"
#include <Adafruit_NeoPixel.h>

// LED Matrix pins (DIN, CLK, LOAD/CS, MaxID)
// pin 12 is connected to DIN
// pin 11 is connected to CLK
// pin 10 is connected to LOAD/CS
LedControl lcMatrix=LedControl(12,11,10,1);

// pin 9 is connected to DIN
// pin 8 is connected to CLK
// pin 7 is connected to LOAD/CS
LedControl lcDigit=LedControl(9,8,7,1);

#define stripRpmPIN 3
#define stripBrakePIN 4
Adafruit_NeoPixel stripRpm = Adafruit_NeoPixel(8, stripRpmPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripBrake = Adafruit_NeoPixel(8, stripBrakePIN, NEO_GRB + NEO_KHZ800);


int maxRPM=0;
int lastRPM=0;
int lastGEAR=0;
//int rnd=0;

float lastSPEED=0;
float lastBRAKE=0;

String myString;
String type;

bool startRead=false;
bool stopRead=false;
byte matrixDC[8] = { B10000001,B01000010,B00100100,B00011000,B00011000,B00100100,B01000010,B10000001 };
byte matrixC[8] = { B00111100,B01100010,B01100000,B01100000,B01100000,B01100000,B01100010,B00111100 };
byte matrixR[8] = { B00111100,B01100110,B01100110,B01100100,B01111000,B01111000,B01101100,B01100110 };
byte matrixN[8] = { B01100110,B01100110,B01110110,B01111110,B01101110,B01100110,B01100110,B01100110 };
byte matrix1[8] = { B00011000,B00111000,B00011000,B00011000,B00011000,B00011000,B00011000,B01111110 };
byte matrix2[8] = { B00111100,B01100110,B00000110,B00001100,B00011000,B00110000,B01111110,B01111110 };
byte matrix3[8] = { B00111100,B01100110,B00000110,B00001100,B00001100,B00000110,B01100110,B00111100 };
byte matrix4[8] = { B00000110,B00001110,B00010110,B00100110,B01100110,B01111111,B00000110,B00000110 };
byte matrix5[8] = { B01111110,B01100000,B01100000,B01111100,B00000110,B00000110,B01100110,B00111100 };
byte matrix6[8] = { B00111100,B01100110,B01100000,B01111100,B01111110,B01100110,B01100110,B00111100 };
byte matrix7[8] = { B01111110,B01000110,B00000110,B00001100,B00001100,B00011000,B00011000,B00011000 };
byte matrix8[8] = { B00111100,B01100110,B01100110,B01111110,B01111110,B01100110,B01100110,B00111100 };
int row;

uint32_t rpmGREEN = stripRpm.Color(128, 255, 0);
uint32_t rpmYELLOW = stripRpm.Color(255, 166, 0);
uint32_t rpmRED = stripRpm.Color(255, 0, 0);
uint32_t rpmOFF = stripRpm.Color(0, 0, 0);
uint32_t rpmBlinkMillis = millis();
uint16_t rpmBlinkDelay = 50;

uint32_t brakeGREEN = stripRpm.Color(128, 255, 0);
uint32_t brakeYELLOW = stripRpm.Color(255, 166, 0);
uint32_t brakeRED = stripRpm.Color(255, 0, 0);
uint32_t brakeOFF = stripRpm.Color(0, 0, 0);

uint32_t dataRXmillis = millis();
uint16_t dataRXth = 1000;
uint32_t dataTXmillis = millis();

bool dataRX = false;

uint8_t toggle=0;


void setup()
{
  Serial.begin(115200);
  myString.reserve(200);

  //RPM Strip
  stripRpm.begin();
  stripRpm.setBrightness(20);
  for(int i=0; i<stripRpm.numPixels(); i++) {
    stripRpm.setPixelColor(i, rpmOFF);
  }
  stripRpm.show();

  //Brake Strip
  stripBrake.begin();
  stripBrake.setBrightness(20);
  for(int i=0; i<stripBrake.numPixels(); i++) {
    stripBrake.setPixelColor(i, brakeOFF);
  }
  stripBrake.show();

  //Gear Matrix
  lcMatrix.shutdown(0,false);// enables display
  lcMatrix.setIntensity(0,5);// sets brightness (0~15 possible values)
  lcMatrix.clearDisplay(0);// clear screen

  //RPM/SPEED Digits
  lcDigit.shutdown(0,false);// enables display
  lcDigit.setIntensity(0,5);// sets brightness (0~15 possible values)
  lcDigit.clearDisplay(0);// clear screen
}


void resetRPM()
{
  lcDigit.setRow(0,7,0x47);
  lcDigit.setRow(0,6,0x1c);
  lcDigit.setRow(0,5,0x0E);
  lcDigit.setRow(0,4,0x0E);
  lcDigit.setChar(0,3,' ',false);
  lcDigit.setRow(0,2,0x0F);
  lcDigit.setRow(0,1,0x17);
  lcDigit.setRow(0,0,0x05);

  //lcDigit.setChar(0,7,'f',false);
  //lcDigit.setRow(0,6,0x1c);
  //lcDigit.setChar(0,5,'l',false);
  //lcDigit.setChar(0,4,'l',false);
  //lcDigit.setChar(0,3,' ',false);
  //lcDigit.setRow(0,2,0x05);
  //lcDigit.setChar(0,1,'p',false);
  //lcDigit.setRow(0,0,0x15);
}


void lcdReady(bool sim=false)
{
  if(sim){
    lcDigit.setRow(0,7,0x05);
    lcDigit.setRow(0,6,0x3D);
    lcDigit.setRow(0,5,0x3B);
    lcDigit.setChar(0,4,' ',false);
    lcDigit.setChar(0,3,' ',false);
    lcDigit.setRow(0,2,0x05);
    lcDigit.setRow(0,1,0x3D);
    lcDigit.setRow(0,0,0x3B);  
  } else {
    lcDigit.setRow(0,7,0x05);
    lcDigit.setRow(0,6,0x3D);
    lcDigit.setRow(0,5,0x3B);
    lcDigit.setChar(0,4,' ',false);
    lcDigit.setChar(0,3,' ',false);
    lcDigit.setChar(0,2,' ',false);
    lcDigit.setChar(0,1,' ',false);
    lcDigit.setChar(0,0,' ',false);
  }
}


void rpmDigit(int value=0)
{
  int ones;
  int tens;
  int hundreds;
  int thousands;

  if(value < 10000){
    ones=value%10;
    value=value/10;
    tens=value%10;
    value=value/10;
    hundreds=value%10;
    value=value/10;
    thousands=value;

    lcDigit.setDigit(0,3,(byte)thousands,false);
    lcDigit.setDigit(0,2,(byte)hundreds,false);
    lcDigit.setDigit(0,1,(byte)tens,false);
    lcDigit.setDigit(0,0,(byte)ones,false);
  } 
  else{
    lcDigit.setDigit(0,3,(byte)9,false);
    lcDigit.setDigit(0,2,(byte)9,false);
    lcDigit.setDigit(0,1,(byte)9,false);
    lcDigit.setDigit(0,0,(byte)9,false);
  }
}


void speedDigit(int value=0)
{
  int ones;
  int tens;
  int hundreds;

  if(value < 1000){
    ones=value%10;
    value=value/10;
    tens=value%10;
    value=value/10;
    hundreds=value%10;
    
    lcDigit.setDigit(0,7,(byte)hundreds,false);
    lcDigit.setDigit(0,6,(byte)tens,false);
    lcDigit.setDigit(0,5,(byte)ones,false);
    lcDigit.setChar(0,4,' ',false);
  } 
  else{
    lcDigit.setDigit(0,7,(byte)9,false);
    lcDigit.setDigit(0,6,(byte)9,false);
    lcDigit.setDigit(0,5,(byte)9,false);
    lcDigit.setChar(0,4,' ',false);
  }
}


void gearMatrix(int value=0)
{         
  switch(value){
    //R
    case -1:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrixR[row]);
        row--;
      }
      break;

    //N  
    case 0:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrixN[row]);
        row--;
      }
      break;

    //1
    case 1:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix1[row]);
        row--;
      }
      break;

    //2  
    case 2:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix2[row]);
        row--;
      }
      break;

    //3  
    case 3:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix3[row]);
        row--;
      }
      break;

    //4  
    case 4:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix4[row]);
        row--;
      }
      break;

    //5  
    case 5:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix5[row]);
        row--;
      }
      break;

    //6
    case 6:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix6[row]);
        row--;
      }
      break;

    //7
    case 7:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix7[row]);
        row--;
      }         
      break;

    //8
    case 8:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrix8[row]);
        row--;
      }    
      break;

    //C
    case 98:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrixC[row]);
        row--;
      }
      break;

    //X
    case 99:
      row = 7;
      for (int column = 0; column < 8; column++) {
        lcMatrix.setColumn(0, column, matrixDC[row]);
        row--;
      }
      break;
  }
}

void rpmStrip(int value=0, int brightness=25)
{
  if (brightness < 16){ brightness = 16; }
  if (brightness > 255){ brightness = 255; }

  if (value == -1){
    toggle = ~toggle;    
    if (toggle)
    {
      stripRpm.setBrightness(brightness);
      //for(int i=0; i<stripRpm.numPixels(); i++) {
      for(int i=stripRpm.numPixels(); i>=0; i--) {
        stripRpm.setPixelColor(i, rpmRED);
      }
      stripRpm.show();
    }else{
      stripRpm.setBrightness(brightness-10);
      for(int i=0; i<stripRpm.numPixels(); i++) {
        stripRpm.setPixelColor(i, rpmRED);
      }
      stripRpm.show();
    }
  } else if (value > ((maxRPM / 100) * 99))
  {

    toggle = ~toggle;    
    if (toggle)
    {
      if ((millis()-rpmBlinkMillis) > rpmBlinkDelay){
        rpmBlinkMillis=millis();
        //rnd=1;
        stripRpm.setBrightness(brightness);
        //for(int i=0; i<stripRpm.numPixels(); i++) {
        for(int i=stripRpm.numPixels(); i>=0; i--) {
          stripRpm.setPixelColor(i, rpmRED);
        }
        stripRpm.show();
      }
    }else{
      if ((millis()-rpmBlinkMillis) > rpmBlinkDelay){
        rpmBlinkMillis=millis();
        //rnd=0;
        stripRpm.setBrightness(brightness);
        //for(int i=0; i<stripRpm.numPixels(); i++) {
        for(int i=stripRpm.numPixels(); i>=0; i--) {
          stripRpm.setPixelColor(i, rpmOFF);
        }
        stripRpm.show();
      }
    }
  }
  //8
  else if (value > ((maxRPM / 100) * 98))
  { 
    stripRpm.setBrightness(brightness-5);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmYELLOW);
    stripRpm.setPixelColor(4, rpmYELLOW);
    stripRpm.setPixelColor(5, rpmYELLOW);
    stripRpm.setPixelColor(6, rpmRED);
    stripRpm.setPixelColor(7, rpmRED);
    stripRpm.show();
  }
  //7
  else if (value > ((maxRPM / 100) * 96))
  {
    stripRpm.setBrightness(brightness-5);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmYELLOW);
    stripRpm.setPixelColor(4, rpmYELLOW);
    stripRpm.setPixelColor(5, rpmYELLOW);
    stripRpm.setPixelColor(6, rpmRED);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //6
  else if (value > ((maxRPM / 100) * 94))
  {   
    stripRpm.setBrightness(brightness-5);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmYELLOW);
    stripRpm.setPixelColor(4, rpmYELLOW);
    stripRpm.setPixelColor(5, rpmYELLOW);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show(); 
  }
  //5
  else if (value > ((maxRPM / 100) * 92))
  {
    stripRpm.setBrightness(brightness-7);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmYELLOW);
    stripRpm.setPixelColor(4, rpmYELLOW);
    stripRpm.setPixelColor(5, rpmOFF);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //4
  else if (value > ((maxRPM / 100) * 90))
  {
    stripRpm.setBrightness(brightness-9);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmYELLOW);
    stripRpm.setPixelColor(4, rpmOFF);
    stripRpm.setPixelColor(5, rpmOFF);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //3
  else if (value > ((maxRPM / 100) * 85))
  {    
    stripRpm.setBrightness(brightness-11);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmGREEN);
    stripRpm.setPixelColor(3, rpmOFF);
    stripRpm.setPixelColor(4, rpmOFF);
    stripRpm.setPixelColor(5, rpmOFF);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //2
  else if (value > ((maxRPM / 100) * 80))
  {
    stripRpm.setBrightness(brightness-13);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmGREEN);
    stripRpm.setPixelColor(2, rpmOFF);
    stripRpm.setPixelColor(3, rpmOFF);
    stripRpm.setPixelColor(4, rpmOFF);
    stripRpm.setPixelColor(5, rpmOFF);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //1
  else if (value > ((maxRPM / 100) * 70))
  {  
    stripRpm.setBrightness(brightness-15);
    stripRpm.setPixelColor(0, rpmGREEN);
    stripRpm.setPixelColor(1, rpmOFF);
    stripRpm.setPixelColor(2, rpmOFF);
    stripRpm.setPixelColor(3, rpmOFF);
    stripRpm.setPixelColor(4, rpmOFF);
    stripRpm.setPixelColor(5, rpmOFF);
    stripRpm.setPixelColor(6, rpmOFF);
    stripRpm.setPixelColor(7, rpmOFF);
    stripRpm.show();
  }
  //0
  else if (value < ((maxRPM / 100) * 70))
  {
    brightness = 1;
      
    stripRpm.setBrightness(brightness);
    //for(int i=0; i<stripRpm.numPixels(); i++) {
    for(int i=stripRpm.numPixels(); i>=0; i--) {
      stripRpm.setPixelColor(i, rpmOFF);
    }
    stripRpm.show();
  }
}


void brakeStrip(float value=0, int brightness=25)
{
  if (brightness < 21){ brightness = 21; }
  if (brightness > 255){ brightness = 255; }

  float maxBRAKE=1.0;

  if (value == -1){
    toggle = ~toggle;    
    if (toggle)
    {
      stripBrake.setBrightness(brightness-10);
      for(int i=0; i<stripBrake.numPixels(); i++) {
        stripBrake.setPixelColor(i, brakeRED);
      }
      stripBrake.show();
    }else{
      stripBrake.setBrightness(brightness);
      for(int i=0; i<stripBrake.numPixels(); i++) {
        stripBrake.setPixelColor(i, brakeRED);
      }
      stripBrake.show();
    }
  }else if (value > ((maxBRAKE / 100) * 96))
  {
    stripBrake.setBrightness(brightness);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeYELLOW);
    stripBrake.setPixelColor(2, brakeYELLOW);
    stripBrake.setPixelColor(1, brakeRED);
    stripBrake.setPixelColor(0, brakeRED);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 84))
  {
    stripBrake.setBrightness(brightness-10);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeYELLOW);
    stripBrake.setPixelColor(2, brakeYELLOW);
    stripBrake.setPixelColor(1, brakeRED);
    stripBrake.setPixelColor(0, brakeRED);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 72))
  {
    stripBrake.setBrightness(brightness-12);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeYELLOW);
    stripBrake.setPixelColor(2, brakeYELLOW);
    stripBrake.setPixelColor(1, brakeRED);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 60))
  {
    stripBrake.setBrightness(brightness-14);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeYELLOW);
    stripBrake.setPixelColor(2, brakeYELLOW);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 48))
  {
    stripBrake.setBrightness(brightness-16);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeYELLOW);
    stripBrake.setPixelColor(2, brakeOFF);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 36))
  {
    stripBrake.setBrightness(brightness-17);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeYELLOW);
    stripBrake.setPixelColor(3, brakeOFF);
    stripBrake.setPixelColor(2, brakeOFF);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 24))
  {
    stripBrake.setBrightness(brightness-18);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeGREEN);
    stripBrake.setPixelColor(4, brakeOFF);
    stripBrake.setPixelColor(3, brakeOFF);
    stripBrake.setPixelColor(2, brakeOFF);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 12))
  {
    stripBrake.setBrightness(brightness-19);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeGREEN);
    stripBrake.setPixelColor(5, brakeOFF);
    stripBrake.setPixelColor(4, brakeOFF);
    stripBrake.setPixelColor(3, brakeOFF);
    stripBrake.setPixelColor(2, brakeOFF);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value > ((maxBRAKE / 100) * 0))
  {
    stripBrake.setBrightness(brightness-20);
    stripBrake.setPixelColor(7, brakeGREEN);
    stripBrake.setPixelColor(6, brakeOFF);
    stripBrake.setPixelColor(5, brakeOFF);
    stripBrake.setPixelColor(4, brakeOFF);
    stripBrake.setPixelColor(3, brakeOFF);
    stripBrake.setPixelColor(2, brakeOFF);
    stripBrake.setPixelColor(1, brakeOFF);
    stripBrake.setPixelColor(0, brakeOFF);
    stripBrake.show();
  }
  else if (value < ((maxBRAKE / 100) * 1))
  {
    brightness = 1;
    
    stripBrake.setBrightness(brightness);
    for(int i=0; i<stripBrake.numPixels(); i++) {
      stripBrake.setPixelColor(i, brakeOFF);
    }
    stripBrake.show();
  }
}


void clearall(bool reset=false)
{
    if(reset)  
      maxRPM=0;
    
    lcDigit.clearDisplay(0);
    lcMatrix.clearDisplay(0);

    for(int i=0; i<stripBrake.numPixels(); i++) {
      stripBrake.setPixelColor(i, brakeOFF);
    }   
    stripBrake.show();

    for(int i=0; i<stripRpm.numPixels(); i++) {
      stripRpm.setPixelColor(i, rpmOFF);
    }   
    stripRpm.show();
}


void noDATA()
{
  gearMatrix(99);
  rpmStrip(-1);
  brakeStrip(-1);

  for(int i=0; i<8; i++) {
    lcDigit.setChar(0,i,'_',false);
  }
    toggle = ~toggle;
    if (toggle) {
      lcDigit.setChar(0,4,'D',false);
      lcDigit.setChar(0,3,'C',false);
    }
}


void loop()
{
  if ((millis()-dataRXmillis) > dataRXth){
    dataRX = false;
  }

  if (dataRX == false) {
    if ((millis()-dataTXmillis) > 1000){
      dataTXmillis=millis();
      noDATA();
      Serial.print("iracing_matrix");
    }
  }

  if (stopRead){
    int mps;
    double kph;
    double mph;
    
    stopRead=false;
    
    if (myString != ""){
      switch(type[0]){
        case 'B':
          lastBRAKE = myString.toFloat();
          brakeStrip(lastBRAKE);
          break;

        case 'G':
          lastGEAR = myString.toInt();
          gearMatrix(lastGEAR);
          break;

        case 'R': 
          lastRPM = myString.toInt();

          if (lastGEAR == 0 && lastSPEED < 10)
          {
            if (lastRPM > maxRPM)
              maxRPM = lastRPM;
          }

          if (maxRPM < 5000)
          {
            resetRPM();
          }
          else
          {
            rpmDigit(lastRPM);
            rpmStrip(lastRPM);
          }
          break;

        case 'S':
          lastSPEED = myString.toFloat();
          mps = myString.toInt();
          kph = lastSPEED * 3.6;
          mph = kph * 5 / 8;

          if (maxRPM < 5000)
          {
            resetRPM();
          }
          else
          {
            speedDigit(kph);
          }     
          break;

        case 'E':
          if (myString.toInt() == 1)
          {
            //Waiting for sim
            clearall(true);
            lcdReady(false);
          }
          else
          {
            //Connected to sim
            clearall(false);
            lcdReady(true);
          }
          break;
      }
    }
  }
}


void serialEvent() 
{
  while (Serial.available()){
    char inChar = (char)Serial.read();
    if (inChar == '!') {
      startRead=true; stopRead=false; myString = "";
      dataRXmillis = millis();
      dataRX = true;
    }else if (inChar == '\n') {
      startRead=false; stopRead=true;
      type = myString[0];
      myString.remove(0,1);
    }else if(startRead){
      myString += inChar;
    }
  }
}
