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

#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskLeitura( void *pvParameters );
void TaskUpdateLCD( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // set up the LCD's number of columns and rows:
  for (int i = 0; i < BUFFER_SIZE; i++) {
    dataDe[i] = 0;
    dataDd[i] = 0;
    dataTe[i] = 0;
    dataTd[i] = 0;
  }

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(4, 0); lcd.print("INICIANDO");

  pinMode(FUNC_PIN, INPUT_PULLUP);
  pinMode(T_PIN, INPUT_PULLUP);
  pinMode(CAL_PIN, INPUT_PULLUP);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskLeitura
    ,  "Leitura"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskUpdateLCD
    ,  "UpdateLCD"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  vTaskStartScheduler();
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskLeitura(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    readButtons();
    readData();
  }
}

void TaskUpdateLCD(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    if ((millis() - latupdateTime) > 1000 / UPDATE_LCD_HZ) {
      latupdateTime = millis();
      state += (funcState ? 1 : 0);
      state = state > 3 ? 0 : state;

      if (tState) {
        tara();
      } else {
        switch (state) {
          case 0:
            printScales();
            break;

          case 1:
            printLat();
            break;

          case 2:
            printLong();
            break;

          case 3:
            printTotal();
            break;

          default:
            break;

        }
        funcState ? vTaskDelay( 100 / portTICK_PERIOD_MS ):vTaskDelay( 1 / portTICK_PERIOD_MS ); 
      }
    }
  }
}


// *********************************************
// CÃ“DIGO BASE DO OUTRO ARQUIVO COM AS FUNÃ‡Ã•ES

//void loop() {
//    readButtons();
//
//    if(calState)
//        calibracao();
//
//#ifdef DEBUG_TIME
//    unsigned long readTime = millis();
//#endif
//    readData();
//
//#ifdef DEBUG_TIME
//    unsigned long readTime_ = millis()-readTime;
//    lcd.clear();
//    lcd.setCursor(0, 0);
//    lcd.print(readTime_);
//#else
//
//#endif
//}
//
void readButtons(){
    funcState = !digitalRead(FUNC_PIN);
    tState = !digitalRead(T_PIN);
    calState = !digitalRead(CAL_PIN);
}

void printScales() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print((de));  //lcd.print(analogRead(FE_PORT)-512);//
  lcd.setCursor(9, 0); lcd.print((dd));  //lcd.print(analogRead(FD_PORT)-512);//
  lcd.setCursor(0, 1); lcd.print((te));  //lcd.print(analogRead(TE_PORT)-512);//
  lcd.setCursor(9, 1); lcd.print((td));  //lcd.print(analogRead(TD_PORT)-512);//

  lcd.setCursor(5, 0); lcd.print("kg");
  lcd.setCursor(5, 1); lcd.print("kg");
  lcd.setCursor(14, 0); lcd.print("kg");
  lcd.setCursor(14, 1); lcd.print("kg");
}
//
void printLong() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Diant.: ");
  lcd.setCursor(0, 1); lcd.print("Tras.: ");
  lcd.setCursor(10, 0); lcd.print(int((de + dd) * 100 / total));
  lcd.setCursor(10, 1); lcd.print(int((te + td) * 100 / total));
  lcd.setCursor(15, 0); lcd.print("%");
  lcd.setCursor(15, 1); lcd.print("%");
}

void printLat() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Esq.: ");
  lcd.setCursor(11, 0); lcd.print("Dir.: ");
  lcd.setCursor(0, 1); lcd.print(int((de + te) * 100 / total));
  lcd.setCursor(11, 1); lcd.print(int((dd + td) * 100 / total));
  lcd.setCursor(4, 1); lcd.print("%");
  lcd.setCursor(15, 1); lcd.print("%");
}

void printTotal() {
  lcd.clear();
  lcd.setCursor(4, 0); lcd.print("Total.: ");
  lcd.setCursor(5, 1); lcd.print(total);
  lcd.setCursor(10, 1); lcd.print("kg");
}

void readData(){
    for(int i = 0; i < BUFFER_SIZE; i++){
        dataDe[i] = analogRead(FE_PORT)-511;
        dataDd[i] = analogRead(FD_PORT)-511;
        dataTe[i] = analogRead(TE_PORT)-511;
        dataTd[i] = analogRead(TD_PORT)-511;
    }
    processData();
}

void tara() {
  lcd.clear();
  lcd.setCursor(0, 4); lcd.print("TARANDO");
  vTaskDelay( 100 / portTICK_PERIOD_MS );
  taraDe += float(de / calibrationFactorDe);
  taraDd += float(dd / calibrationFactorDd);
  taraTe += float(te / calibrationFactorTe);
  taraTd += float(td / calibrationFactorTd);
}
//
//
void processData(){
    de = (float(mediaMovel(dataDe))-taraDe)*calibrationFactorDe;
    dd = (float(mediaMovel(dataDd))-taraDd)*calibrationFactorDd;
    te = (float(mediaMovel(dataTe))-taraTe)*calibrationFactorTe;
    td = (float(mediaMovel(dataTd))-taraTd)*calibrationFactorTd;
    total = de+dd+te+td;
    total = total==0?1:total;
}

int32_t mediaMovel(int32_t *array){
    int32_t media = 0;
    for(int i = 0; i<BUFFER_SIZE; i++){
        media += (array[i]);
    }
    return (media >> bitshift);// / BUFFER_SIZE;
}
