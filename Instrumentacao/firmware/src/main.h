//
// Created by eugen on 22/08/2021.
//

#ifndef FIRMWARE_MAIN_H
#define FIRMWARE_MAIN_H

#endif //FIRMWARE_MAIN_H

/*  The circuit:
  * LCD RS pin to digital pin 12
  * LCD Enable pin to digital pin 11
  * LCD D4 pin to digital pin 5
  * LCD D5 pin to digital pin 4
  * LCD D6 pin to digital pin 3
  * LCD D7 pin to digital pin 2
  * LCD R/W pin to ground
  * LCD VSS pin to ground
  * LCD VCC pin to 5V
  * 10K resistor:
  * ends to +5V and ground
  * wiper to LCD VO pin (pin 3)

  Library originally added 18 Apr 2008
  by David A. Mellis
  library modified 5 Jul 2009
  by Limor Fried (http://www.ladyada.net)
  example added 9 Jul 2009
  by Tom Igoe
  modified 22 Nov 2010
  by Tom Igoe
  modified 7 Nov 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

  */

// include the library code:
#include <LiquidCrystal.h>

#define FE_PORT 0
#define FD_PORT 1
#define TE_PORT 2
#define TD_PORT 3

#define FUNC_PIN   2
#define T_PIN      3
#define CAL_PIN    4

#define BUFFER_SIZE 16
#define UPDATE_LCD_HZ 10
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const double calibrationFactor = 0.5594;
unsigned long latupdateTime = 0;

byte state = 0;
boolean funcState = false;
boolean tState = false;
boolean calState = false;

uint16_t dataDe[BUFFER_SIZE];
uint16_t dataDd[BUFFER_SIZE];
uint16_t dataTe[BUFFER_SIZE];
uint16_t dataTd[BUFFER_SIZE];

float taraDe = 0;
float taraDd = 0;
float taraTe = 0;
float taraTd = 0;

float total = 0;
float de = 0;
float dd = 0;
float te = 0;
float td = 0;

void printScales();
void printLong();
void printLat();
void printTotal();
void tara();

void readButtons();
void readData();
void processData();
float mediaMovel(uint16_t *array);