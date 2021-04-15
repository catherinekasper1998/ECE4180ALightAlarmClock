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

//Modes
#define SLEEP       0
#define COLOR_WHEEL 1
#define RAINBOW     2
#define LIGHT_ON    3
#define LIGHT_OFF   4

//Colors
#define RED     0xFF0000
#define ORANGE  0xFFFFFF
#define YELLOW  0x000000
#define GREEN   0x00FF00
#define BLUE    0x0000FF
#define PURPLE  0xBFBFBF
#define WHITE   0xFFFFFF
#define BLACK   0x000000
#define LGREY   0xBFBFBF
#define DGREY   0x5F5F5F

//Global Settings
time_t LOCAL_TIME;
time_t ALARM_TIME;
int SNOOZE_DURATION_MIN = 5;
int SUNRISE_AND_SUNSET_DURATION_MIN = 30;
int CURRENT_MODE = OFF;
int RAINBOW_COLOR = WHITE;


//page that allows the user to edit all of the different 
void viewSettingsScreen() {

    // Set up
    uLCD.cls();
    uLCD.color(WHITE);
    uLCD.text_width(1); // size text
    uLCD.text_height(1.25);
    uLCD.locate(0,0);
    uLCD.printf("VIEW SETTINGS\n\n");

    //ALARM TIME Line 1
    uLCD.printf("Alarm: ");
    //UPDATE WITH VARS
    uLCD.printf("00:00");
    uLCD.printf("am");
    uLCD.printf("\n\n");
    
    //LOCAL TIME Line 2
    uLCD.printf("Local: ");
    //UPDATE WITH VARS
    uLCD.printf("00:00");
    uLCD.printf("am");
    uLCD.printf("\n\n");
    
    //SNOOZE DURATION Line 3
    uLCD.printf("Snooze Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("00");
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 4
    uLCD.printf("Sunset Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("30");
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 5
    uLCD.printf("Mode: ");
    //UPDATE WITH VARS
    uLCD.printf("SLEEP");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Bottom Line
    uLCD.locate(0,15);
    uLCD.printf("Back");
    //UPDATE WITH VARS
}

//page that allows the user to edit all of the different 
void changeSettingsScreen() {
    // Set up
    uLCD.cls();
    uLCD.color(WHITE);
    uLCD.text_width(1); // size text
    uLCD.text_height(1.25);
    uLCD.locate(0,0);
    uLCD.printf("CHANGE SETTINGS\n\n");

    //ALARM TIME Line 1
    uLCD.printf("Alarm: ");
    //UPDATE WITH VARS
    uLCD.printf("00:00");
    uLCD.printf("am");
    uLCD.printf("\n\n");
    
    //LOCAL TIME Line 2
    uLCD.printf("Local: ");
    //UPDATE WITH VARS
    uLCD.printf("00:00");
    uLCD.printf("am");
    uLCD.printf("\n\n");
    
    //SNOOZE DURATION Line 3
    uLCD.printf("Snooze Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("00");
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 4
    uLCD.printf("Sunset Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("30");
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 5
    uLCD.printf("Mode: ");
    //UPDATE WITH VARS
    uLCD.printf("SLEEP");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Bottom Line
    uLCD.locate(0,15);
    uLCD.printf("Back");
    //UPDATE WITH VARS
}

void menuScreen() {
    
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
    changeSettingsScreen();
    while(1) {

    }
}

