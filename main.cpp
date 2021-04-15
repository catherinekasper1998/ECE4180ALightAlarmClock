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

//enums
typedef enum {
    SLEEP           = 0;
    COLOR_WHEEL     = 1;
    RAINBOW         = 2;
    LIGHT           = 3;
    OFF             = 4:
} Mode;

typedef enum {
    RED             = 0xFF0000;
    ORGANGE         = 0xFFA500;
    YELLOW          = 0xFFFF00;
    GREEN           = 0x00FF00;
    BLUE            = 0x0000FF;
    PURPLE          = 0xFF00FF;
    PINK            = 0xFF00FF;
    WHITE           = 0xFFFFFF;
} Color;


//Global Settings
time_t LOCAL_TIME;
time_t ALARM_TIME;
int SNOOZE_DURATION_MIN = 5;
int SUNRISE_AND_SUNSET_DURATION_MIN = 30;
Mode CURRENT_MODE = OFF;
Color RAINBOW_COLOR = WHITE;


//screen options


void viewSettings() {

}

//page that allows the user to edit all of the different 
void changeSettings() {
    
}


void homeScreen(){
    uLCD.cls();
    uLCD.color(WHITE);
    //ALARM
    uLCD.text_width(1.25); //4X size text
    uLCD.text_height(1.25);
    uLCD.locate(0,0);
    uLCD.printf("Alarm: ");
    //UPDATE WITH VARS
    uLCD.printf("00:00");
    uLCD.printf("am");
    
    //TIME
    uLCD.text_width(2.5); //4X size text
    uLCD.text_height(2.5);
    uLCD.locate(1.5,2.5);
    //UPDATE WITH VARS
    uLCD.printf("12:00");
    uLCD.printf("am");
    
    //MENU/MODE
    uLCD.text_width(1.25); //4X size text
    uLCD.text_height(1.25);
    uLCD.locate(2,14);
    uLCD.printf("Menu");
    uLCD.locate(9,14);
    uLCD.printf("Mode: ");
    //GET MODE
    uLCD.printf("mode");
}

int main() {
    uLCD.cls();
    uLCD.baudrate(BAUD_3000000); //jack up baud rate to max for fast display
    wait(1.0);
    homeScreen();
    while(1) {

    }
}
