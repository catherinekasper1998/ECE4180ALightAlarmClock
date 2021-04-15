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
    uLCD.locate(1,2.5);
    //Pull current time and set it to a charArray
    char curTime[32];
    strftime(curTime, 32, "%I:%M %p\n", localtime(&LOCAL_TIME));
    uLCD.printf("%s", curTime);
    //DEBUG
    printf("\r\n%s", curTime);
    //END DEBUG
    //MENU/MODE
    uLCD.text_width(1.25); //4X size text
    uLCD.text_height(1.25);
    uLCD.locate(1,14);
    uLCD.printf("Menu");
    uLCD.locate(8,14);
    uLCD.printf("Mode: ");
    //GET MODE
    uLCD.printf("mode");
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

