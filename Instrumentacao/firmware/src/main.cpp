#include <Arduino.h>

#include "main.h"

void setup() {
    // set up the LCD's number of columns and rows:
    for(int i = 0; i < BUFFER_SIZE; i++){
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
}

void loop() {
    readButtons();

    if(calState)
        calibracao();

#ifdef DEBUG_TIME
    unsigned long readTime = millis();
#endif
    readData();

#ifdef DEBUG_TIME
    unsigned long readTime_ = millis()-readTime;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(readTime_);
#else
    if((millis() - latupdateTime) > 1000/UPDATE_LCD_HZ){
        latupdateTime = millis();
        state += (funcState? 1:0);
        state = state > 3? 0: state;

        if(tState){
            tara();
        }else{
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
            funcState? delay(100):delay(1);
        }
    }
#endif
}

void readButtons(){
    funcState = !digitalRead(FUNC_PIN);
    tState = !digitalRead(T_PIN);
    calState = !digitalRead(CAL_PIN);
}

void printScales(){
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

void printLong(){
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Diant.: ");
    lcd.setCursor(0, 1); lcd.print("Tras.: ");
    lcd.setCursor(10, 0); lcd.print(int((de+dd)*100/total));
    lcd.setCursor(10, 1); lcd.print(int((te+td)*100/total));
    lcd.setCursor(15, 0); lcd.print("%");
    lcd.setCursor(15, 1); lcd.print("%");
}

void printLat(){
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Esq.: ");
    lcd.setCursor(11, 0); lcd.print("Dir.: ");
    lcd.setCursor(0, 1); lcd.print(int((de+te)*100/total));
    lcd.setCursor(11, 1); lcd.print(int((dd+td)*100/total));
    lcd.setCursor(4, 1); lcd.print("%");
    lcd.setCursor(15, 1); lcd.print("%");
}

void printTotal(){
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

void tara(){
    lcd.clear();
    lcd.setCursor(0, 4); lcd.print("TARANDO");
    delay(100);
    taraDe += float(de/calibrationFactorDe);
    taraDd += float(dd/calibrationFactorDd);
    taraTe += float(te/calibrationFactorTe);
    taraTd += float(td/calibrationFactorTd);
}


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

void calibracao(){
    //TODO: IMPLEMENTAR CALIBRAÇÃO
}