#include "mbed.h"
#include "mbed.h"
#include "uLCD_4DGL.h"

/*
Final Project for 4180.
A light alarm clock with custom sunrise/sunset settings

*/
//LCD screen
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;

//mbed LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

//pushbuttons
DigitalIn snoozePB(p8);
DigitalIn sleepPB(p15);
DigitalIn upPB(p16);
DigitalIn downPB(p17);
DigitalIn leftPB(p18);
DigitalIn rightPB(p19);
DigitalIn centerPB(p20);


int main() {
    uLCD.cls();
    uLCD.baudrate(BAUD_3000000); //jack up baud rate to max for fast display
    wait(1.0);
    while(1) {

    }
}
