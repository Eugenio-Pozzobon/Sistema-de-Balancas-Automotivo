//
// Created by eugen on 26/08/2021.
//

#ifndef FIRMWARERTOS_MAIN_H
#define FIRMWARERTOS_MAIN_H
#endif FIRMWARERTOS_MAIN_H

// include the library code:
#include <LiquidCrystal.h>
//#define DEBUG_TIME
#define FE_PORT 0
#define FD_PORT 1
#define TE_PORT 2
#define TD_PORT 3

#define FUNC_PIN   2
#define T_PIN      3
#define CAL_PIN    4

#define BUFFER_SIZE 32
#define bitshift    5
#define UPDATE_LCD_HZ 25
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const double calibrationFactorDe = 0.48481;
const double calibrationFactorDd = 0.48481;
const double calibrationFactorTe = 0.48481;
const double calibrationFactorTd = 0.48481;

unsigned long latupdateTime = 0;

byte state = 0;
boolean funcState = false;
boolean tState = false;
boolean calState = false;

int32_t dataDe[BUFFER_SIZE];
int32_t dataDd[BUFFER_SIZE];
int32_t dataTe[BUFFER_SIZE];
int32_t dataTd[BUFFER_SIZE];

double taraDe = 0;
double taraDd = 0;
double taraTe = 0;
double taraTd = 0;

double total = 0;
double de = 0;
double dd = 0;
double te = 0;
double td = 0;

//void printScales();
//void printLong();
//void printLat();
//void printTotal();
//void tara();
//void calibracao();
//
//void readButtons();
//void readData();
//void processData();
//int32_t mediaMovel(int32_t *array);