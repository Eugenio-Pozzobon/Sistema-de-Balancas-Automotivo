//
// Created by eugen on 26/08/2021.
//

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>

/// defines analog inputs pin
#define FE_PORT 0
#define FD_PORT 1
#define TE_PORT 2
#define TD_PORT 3

/// defines digital inputs pin
#define FUNC_PIN   2
#define T_PIN      3

/// defines constants of buffer and media movel calculation
#define BUFFER_SIZE 32
#define bitshift    5
#define UPDATE_LCD_HZ 10

// initialize the LCD by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/// initialize calibration data that converts Voltage (0-1023) to Kg
const double calibrationFactorDe = 0.48481;
const double calibrationFactorDd = 0.48481;
const double calibrationFactorTe = 0.48481;
const double calibrationFactorTd = 0.48481;

/// state of LCD (witch function is beeing displayed)
byte state = 0;

/// state of each button
boolean funcState = false;
boolean tState = false;

/// data buffers
int32_t dataDe[BUFFER_SIZE];
int32_t dataDd[BUFFER_SIZE];
int32_t dataTe[BUFFER_SIZE];
int32_t dataTd[BUFFER_SIZE];

/// data constants
int32_t taraDe = 0;
int32_t taraDd = 0;
int32_t taraTe = 0;
int32_t taraTd = 0;

double total = 0;
double de = 0;
double dd = 0;
double te = 0;
double td = 0;

/// functions
void printScales();
void printLong();
void printLat();
void printTotal();
void tara();

void readButtons();
void readData();
void processData();
int32_t mediaMovel(int32_t *array);