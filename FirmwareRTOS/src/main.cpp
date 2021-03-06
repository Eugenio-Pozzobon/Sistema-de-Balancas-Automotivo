#include <Arduino.h>

#include "main.h"

///include FreeRTOS librarys
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>

/// define two tasks for read and print data
[[noreturn]] void TaskLeitura(void *pvParameters);
[[noreturn]] void TaskUpdateLCD(void *pvParameters);

SemaphoreHandle_t threadSemaphore; ///mutex que vai controlar a thread

void setup() {

    /// initialize serial communication at 9600 bits per second:
    Serial.begin(9600);

    /// read EEPROM data
    taraDe = EEPROM.read(0) << 8 | EEPROM.read(1);
    taraDd = EEPROM.read(2) << 8 | EEPROM.read(3);
    taraTe = EEPROM.read(4) << 8 | EEPROM.read(5);
    taraTd = EEPROM.read(6) << 8 | EEPROM.read(7);

    if (threadSemaphore == NULL) {
        threadSemaphore = xSemaphoreCreateMutex();  ///cria a mutex
    }
    if ((threadSemaphore) != NULL)
        xSemaphoreGive((threadSemaphore));

    /// Now set up two tasks to run independently.
    xTaskCreate(
            TaskLeitura, "Leitura"   // A name just for humans
            , 128  // This stack size can be checked & adjusted by reading the Stack Highwater
            , NULL, 1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
            , NULL);

    xTaskCreate(
            TaskUpdateLCD, "UpdateLCD", 128  // Stack size
            , NULL, 2  // Priority
            , NULL);

    vTaskStartScheduler();
}

void loop() {
    // Empty. Things are done in Tasks.
}

[[noreturn]] void TaskLeitura(void *pvParameters) {
    (void) pvParameters;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        dataDe[i] = 0;
        dataDd[i] = 0;
        dataTe[i] = 0;
        dataTd[i] = 0;
    }

    pinMode(FUNC_PIN, INPUT_PULLUP);
    pinMode(T_PIN, INPUT_PULLUP);

    while (true) { // A Task shall never return or exit.
        if (xSemaphoreTake(threadSemaphore, (TickType_t) 5) == pdTRUE) {
            readButtons();
            readData();
            xSemaphoreGive(threadSemaphore);
        }
    }
}

[[noreturn]] void TaskUpdateLCD(void *pvParameters)  // This is a task.
{
    (void) pvParameters;


    lcd.begin(16, 2);
    lcd.clear();

    lcd.setCursor(4, 0);
    lcd.print("INICIANDO");


    while (true) {
        if (xSemaphoreTake(threadSemaphore, (TickType_t) 5) == pdTRUE) {
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
            }
            xSemaphoreGive(threadSemaphore);
            vTaskDelay((1000 / UPDATE_LCD_HZ) / portTICK_PERIOD_MS);
        }
    }
}


/// read state of each button
void readButtons() {
    funcState = !digitalRead(FUNC_PIN);
    tState = !digitalRead(T_PIN);
}


/// print data in LCD about individual scales data
void printScales() {
    lcd.clear();

    //print values
    lcd.setCursor(0, 0);
    lcd.print((de));
    lcd.setCursor(9, 0);
    lcd.print((dd));
    lcd.setCursor(0, 1);
    lcd.print((te));
    lcd.setCursor(9, 1);
    lcd.print((td));

    //print units
    lcd.setCursor(5, 0);
    lcd.print("kg");
    lcd.setCursor(5, 1);
    lcd.print("kg");
    lcd.setCursor(14, 0);
    lcd.print("kg");
    lcd.setCursor(14, 1);
    lcd.print("kg");
}

/// print data in LCD about longitudinal mass distribution
void printLong() {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Diant.: ");
    lcd.setCursor(0, 1);
    lcd.print("Tras.: ");

    //print values
    lcd.setCursor(10, 0);
    lcd.print(int((de + dd) * 100 / total));
    lcd.setCursor(10, 1);
    lcd.print(int((te + td) * 100 / total));
    lcd.setCursor(15, 0);

    //print units
    lcd.print("%");
    lcd.setCursor(15, 1);
    lcd.print("%");
}

/// print data in LCD about lateral mass distribution
void printLat() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Esq.: ");
    lcd.setCursor(11, 0);
    lcd.print("Dir.: ");
    lcd.setCursor(0, 1);
    lcd.print(int((de + te) * 100 / total));
    lcd.setCursor(11, 1);
    lcd.print(int((dd + td) * 100 / total));
    lcd.setCursor(4, 1);
    lcd.print("%");
    lcd.setCursor(15, 1);
    lcd.print("%");
}

/// print data in LCD about total mass of the car
void printTotal() {
    lcd.clear();

    //print values
    lcd.setCursor(4, 0);
    lcd.print("Total.: ");
    lcd.setCursor(5, 1);
    lcd.print(total);

    //print units
    lcd.setCursor(10, 1);
    lcd.print("kg");
}

/// read analog data and store its in an vector
void readData() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        dataDe[i] = analogRead(FE_PORT) - 511;
        dataDd[i] = analogRead(FD_PORT) - 511;
        dataTe[i] = analogRead(TE_PORT) - 511;
        dataTd[i] = analogRead(TD_PORT) - 511;
    }
    processData();
}


/// get the current analog data and store its to EEPROM
/// this value will be subtract from de current data that is being redden
void tara() {
    lcd.clear();
    lcd.setCursor(0, 4);
    lcd.print("TARANDO");
    taraDe += int32_t(de / calibrationFactorDe);
    taraDd += int32_t(dd / calibrationFactorDd);
    taraTe += int32_t(te / calibrationFactorTe);
    taraTd += int32_t(td / calibrationFactorTd);

    EEPROM.write(0,taraDe >> 8);
    EEPROM.write(1,taraDe);
    EEPROM.write(2,taraDd >> 8);
    EEPROM.write(3,taraDd);
    EEPROM.write(4,taraTe >> 8);
    EEPROM.write(5,taraTe);
    EEPROM.write(6,taraTd >> 8);
    EEPROM.write(7,taraTd);
}

/// process data, calculating media movel,
/// removing tara, and converting to kg
void processData() {
    de = (float(mediaMovel(dataDe) - taraDe)) * calibrationFactorDe;
    dd = (float(mediaMovel(dataDd) - taraDd)) * calibrationFactorDd;
    te = (float(mediaMovel(dataTe) - taraTe)) * calibrationFactorTe;
    td = (float(mediaMovel(dataTd) - taraTd)) * calibrationFactorTd;
    total = de + dd + te + td;
    total = total == 0 ? 1 : total;
}

/// calculate media movel of a vector
int32_t mediaMovel(int32_t *array) {
    int32_t media = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        media += (array[i]);
    }
    return (media >> bitshift);
}