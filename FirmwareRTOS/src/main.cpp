#include <Arduino.h>

#include "main.h"

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>

// define two tasks for read and print data
[[noreturn]] void TaskLeitura(void *pvParameters);

[[noreturn]] void TaskUpdateLCD(void *pvParameters);

SemaphoreHandle_t xSerialSemaphore; ///mutex que vai controlar porta serial

void setup() {

    // initialize serial communication at 9600 bits per second:
    Serial.begin(9600);

    taraDe = EEPROM.read(0) << 8 | EEPROM.read(1);
    taraDd = EEPROM.read(2) << 8 | EEPROM.read(3);
    taraTe = EEPROM.read(4) << 8 | EEPROM.read(5);
    taraTd = EEPROM.read(6) << 8 | EEPROM.read(7);

    if (xSerialSemaphore == NULL) { ///verifica se o semaforo da porta serial ja existe
        xSerialSemaphore = xSemaphoreCreateMutex();  ///cria a mutex que controla a porta serial
    }


    if ((xSerialSemaphore) != NULL)
        xSemaphoreGive((xSerialSemaphore));  ///torna a porta serial disponivel

    // Now set up two tasks to run independently.
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
    // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
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
    pinMode(CAL_PIN, INPUT_PULLUP);

    while (true) { // A Task shall never return or exit.
        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
            readButtons();
            readData();
            xSemaphoreGive(xSerialSemaphore);
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
        if (xSemaphoreTake(xSerialSemaphore, (TickType_t) 5) == pdTRUE) {
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
            xSemaphoreGive(xSerialSemaphore);
            vTaskDelay((1000 / UPDATE_LCD_HZ) / portTICK_PERIOD_MS);
        }
    }
}

void readButtons() {
    funcState = !digitalRead(FUNC_PIN);
    tState = !digitalRead(T_PIN);
    calState = !digitalRead(CAL_PIN);
}

void printScales() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print((de));  //lcd.print(analogRead(FE_PORT)-512);//
    lcd.setCursor(9, 0);
    lcd.print((dd));  //lcd.print(analogRead(FD_PORT)-512);//
    lcd.setCursor(0, 1);
    lcd.print((te));  //lcd.print(analogRead(TE_PORT)-512);//
    lcd.setCursor(9, 1);
    lcd.print((td));  //lcd.print(analogRead(TD_PORT)-512);//

    lcd.setCursor(5, 0);
    lcd.print("kg");
    lcd.setCursor(5, 1);
    lcd.print("kg");
    lcd.setCursor(14, 0);
    lcd.print("kg");
    lcd.setCursor(14, 1);
    lcd.print("kg");
}

///
void printLong() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Diant.: ");
    lcd.setCursor(0, 1);
    lcd.print("Tras.: ");
    lcd.setCursor(10, 0);
    lcd.print(int((de + dd) * 100 / total));
    lcd.setCursor(10, 1);
    lcd.print(int((te + td) * 100 / total));
    lcd.setCursor(15, 0);
    lcd.print("%");
    lcd.setCursor(15, 1);
    lcd.print("%");
}

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

void printTotal() {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Total.: ");
    lcd.setCursor(5, 1);
    lcd.print(total);
    lcd.setCursor(10, 1);
    lcd.print("kg");
}

void readData() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        dataDe[i] = analogRead(FE_PORT) - 511;
        dataDd[i] = analogRead(FD_PORT) - 511;
        dataTe[i] = analogRead(TE_PORT) - 511;
        dataTd[i] = analogRead(TD_PORT) - 511;
    }
    processData();
}


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

void processData() {
    de = (float(mediaMovel(dataDe) - taraDe)) * calibrationFactorDe;
    dd = (float(mediaMovel(dataDd) - taraDd)) * calibrationFactorDd;
    te = (float(mediaMovel(dataTe) - taraTe)) * calibrationFactorTe;
    td = (float(mediaMovel(dataTd) - taraTd)) * calibrationFactorTd;
    total = de + dd + te + td;
    total = total == 0 ? 1 : total;
}

int32_t mediaMovel(int32_t *array) {
    int32_t media = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        media += (array[i]);
    }
    return (media >> bitshift);// / BUFFER_SIZE;
}