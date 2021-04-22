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

// Pages & Lines
#define MAIN        6
#define MENU        7
#define CHANGE_SETTINGS 8
#define VIEW_SETTINGS   9
int page = MAIN;
int line = 0; // value from 0 to n-1 where n = the number of lines on the menu page


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


//screen options


void viewSettingsScreen() {

}

//page that allows the user to edit all of the different 
void changeSettingsScreen() {
    
}

void menuScreen() {
    uLCD.color(WHITE);
    //VIEW SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,1);
    uLCD.printf("View Settings");
    //CHANGE SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,4);
    uLCD.printf("Change Settings");
    //BACK
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,14);
    uLCD.printf("Back");
    //CURSOR
    //NEED SOMETHING FOR SELECTING
}


void homeScreen(){
    
        uLCD.color(WHITE);
    //VIEW SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,1);
    uLCD.printf("View Settings");
    //CHANGE SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,4);
    uLCD.printf("Change Settings");
    //BACK
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,14);
    uLCD.printf("Back");
    //CURSOR
    //NEED SOMETHING FOR SELECTING
}


void homeScreen(){
    
    uLCD.color(WHITE);
    //ALARM
    uLCD.text_width(1.25); 
    uLCD.text_height(1.25);
    uLCD.locate(0,0);
    uLCD.printf("Alarm: ");
    //UPDATE WITH VARS
    char alarmTime[32];
    strftime(alarmTime, 32, "%I:%M %p\n", localtime(&ALARM_TIME));
    uLCD.printf("%s", alarmTime);
    
    //TIME
    uLCD.text_width(2.5); 
    uLCD.text_height(2.5);
    uLCD.locate(0.5,2.5);
    //Pull current time and set it to a charArray
    char curTime[32];
    strftime(curTime, 32, "%I:%M %p\n", localtime(&LOCAL_TIME));
    uLCD.printf("%s", curTime);

    //MENU/MODE
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,12);
    uLCD.printf("Menu");
    uLCD.locate(1,14);
    uLCD.printf("Mode:");
    
    //GET MODE
    uLCD.locate(6,14);
    switch(CURRENT_MODE){
        case SLEEP:
            uLCD.printf("Sleep");
            break;
        case COLOR_WHEEL:
            uLCD.printf("Color Wheel");
            break;
        case RAINBOW:
            uLCD.printf("Rainbow");
            break;
        case LIGHT_ON:
            uLCD.printf("Light On");
            break;
        case LIGHT_OFF:
            uLCD.printf("Light Off");
            break;
        default:
            uLCD.printf("n/a");
            break;
    }
}

void editVariable(){
    //ALARM AND LOCAL
    //hour location
    uLCD.locate(8,2);
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(8,2);
    uLCD.printf("12");
    wait(0.5);
    
    //min location
    uLCD.locate(11,2);
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(11,2);
    uLCD.printf("59");
    wait(0.5);
    
    //am/pm
    uLCD.locate(13,2);
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(13,2);
    uLCD.printf("am");
    wait(0.5);
}

int main() {
    uLCD.cls();
    uLCD.baudrate(BAUD_3000000); //jack up baud rate to max for fast display
    wait(1.0);
    
    set_time(0);
    
    while(1) {
        LOCAL_TIME = time(NULL);
        wait(1.0);
        homeScreen();
    }
}

