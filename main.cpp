#include "mbed.h"
#include "uLCD_4DGL.h"
#include "rtos.h"
#include "TI_NEOPIXEL.h"

/*
Final Project for 4180.
A light alarm clock with custom sunrise/sunset settings
*/
//LCD screen
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;
Serial pc(USBTX, USBRX); // tx, rx
Serial blue(p28,p27);
Thread thread;

TI_NEOPIXEL var(p13);
int ring_number = 12;
int start = 0;
int finish = 12;
//ADD TO MAIN BRANCH
Thread speakerThread;
Thread sunriseThread;
//speaker
PwmOut speaker(p21);


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
#define PINK    0xFF01ED
#define WHITE   0xFFFFFF
#define BLACK   0x000000
#define LGREY   0xBFBFBF
#define BLUETOOTH   0x5F5F5F // Actually Dark Grey

//Global Settings
time_t LOCAL_TIME;
time_t ALARM_TIME = 300;
int SNOOZE_DURATION_MIN = 5;
int SUNRISE_AND_SUNSET_DURATION_MIN = 30;
volatile int CURRENT_MODE = SLEEP;
int RAINBOW_COLOR = WHITE;
volatile int COLOR_WHEEL_COLOR = RED; // this is a state variable 
volatile bool USE_BLUETOOTH = false;
volatile int BLUETOOTH_COLOR = WHITE; // This is what bluetooth color wheel color should be set yo
volatile int BLUETOOTH_RED = 0;
volatile int BLUETOOTH_GREEN = 0;
volatile int BLUETOOTH_BLUE = 0;

int cursor_x = 4;
int cursor_y = 19;
int cursor_radius = 2;
int cursor_color = RED;

char* getCurrentMode() {
    switch(CURRENT_MODE){
        case SLEEP:
            return "Sleep";
        case COLOR_WHEEL:
            return "Color Wheel";
        case RAINBOW:
            return "Rainbow";
        case LIGHT_ON:
            return "Light On";
        case LIGHT_OFF:
            return "Light Off";
        default:
            return "n/a";
    }
}

char* getCurrentColorWheel() {
    switch(COLOR_WHEEL_COLOR){
        case RED:
            return "RED     ";
        case ORANGE:
            return "ORANGE  ";
        case YELLOW:
            return "YELLOW  ";
        case GREEN:
            return "GREEN   ";
        case BLUE:
            return "BLUE    ";
        case PURPLE:
            return "PURPLE  ";
        case PINK:
            return "PINK    ";
        case BLUETOOTH:
            return "BT CW   ";
        default:
            return "n/a    ";
        }
}

void changeColorWheel(int upOrDown) { // up is a + number, down is a - number
    switch (COLOR_WHEEL_COLOR) {
        case RED:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = BLUETOOTH;
            else COLOR_WHEEL_COLOR = ORANGE;
            USE_BLUETOOTH = false;
            break;
        case ORANGE:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = RED;
            else COLOR_WHEEL_COLOR = YELLOW;
            break;
        case YELLOW:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = ORANGE;
            else COLOR_WHEEL_COLOR = GREEN;
            break;
        case GREEN:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = YELLOW;
            else COLOR_WHEEL_COLOR = BLUE;
            break;
        case BLUE:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = GREEN;
            else COLOR_WHEEL_COLOR = PURPLE;
            break;
        case PURPLE:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = BLUE;
            else COLOR_WHEEL_COLOR = PINK;
            break;
        case PINK:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = PURPLE;
            else COLOR_WHEEL_COLOR = BLUETOOTH;
            USE_BLUETOOTH = false;
            break;
        case BLUETOOTH:
            if (upOrDown < 0) COLOR_WHEEL_COLOR = PINK;
            else COLOR_WHEEL_COLOR = RED;
            USE_BLUETOOTH = true;
            break;
        default: 
            // Should not occur
            break;
    }

}

void updateCursor(){ 
    // UPDATE THE PAGE AND INDEX BEFORE CALLING THIS FUNCTION
    // This function does handle line being "out of bounds" 
    // and reassigns a "wrapped around value" its proper line

    uLCD.filled_circle(cursor_x, cursor_y, cursor_radius, BLACK);

    switch (page) {

        case MAIN:              // line should ever only be 0
            Thread::wait(500);      // flash the circle so people know that it is active
            cursor_x = 4;        // does not change
            cursor_y = 99;        // does not change
            line = 0;
            break;
        
        case MENU:
            if (line == 0 | line > 2) {            // View Settings
                cursor_x = 4;
                cursor_y = 19;
                line = 0;
            } else if (line == 1) {     // Change Settings
                cursor_x = 4;
                cursor_y = 19 + 24;
            } else if (line == 2 | line < 0) {     // Back
                cursor_x = 4;
                cursor_y = 123;
                line = 2;
            } else {
                // ERROR!
            }
            break;
        
        case CHANGE_SETTINGS:
            if (line == 0 | line > 6) { // Alarm Time
                cursor_x = 4;
                cursor_y = 19;
                line = 0;
            } else if (line == 1) {     // Snooze Time
                cursor_x = 4;
                cursor_y = 19 + 16;
            } else if (line == 2) {     // Local Time
                cursor_x = 4;
                cursor_y = 19 + 16 * 2;
            } else if (line == 3) {     // Sunrise/Sunset Time
                cursor_x = 4;
                cursor_y = 19 + 16 * 3;
            } else if (line == 4) {     // Current Mode
                cursor_x = 4;
                cursor_y = 19 + 16 * 4;
            } else if (line == 5) {     // Color Wheel Color
                cursor_x = 4;
                cursor_y = 19 + 16 * 5;
            } else if (line == 6 | line < 0) {     // Back & Save
                cursor_x = 4;
                cursor_y = 122;
                line = 6;
            }
            break;
        
        case VIEW_SETTINGS: // SHOULD ONLY EVER BE 0
            Thread::wait(500);
            cursor_y = 3;
            cursor_y = 123;
            line = 0;
            break;
        
        default:
            break;
    }
    uLCD.filled_circle(cursor_x, cursor_y, cursor_radius, cursor_color);

}


//page that allows the user to edit all of the different 
void viewSettingsScreen() {
    page = VIEW_SETTINGS;
    line = 0;

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
    char alarmTime[32];
    strftime(alarmTime, 32, "%I:%M %p", localtime(&ALARM_TIME));
    uLCD.printf("%s", alarmTime);
    uLCD.printf("\n\n");
    
    //LOCAL TIME Line 2
    uLCD.printf("Local: ");
    //UPDATE WITH VARS
    char curTime[32];
    strftime(curTime, 32, "%I:%M %p", localtime(&LOCAL_TIME));
    uLCD.printf("%s", curTime);
    uLCD.printf("\n\n");
    
    //SNOOZE DURATION Line 3
    uLCD.printf("Snooze Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("%d", SNOOZE_DURATION_MIN);
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 4
    uLCD.printf("Sunset Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("%d", SUNRISE_AND_SUNSET_DURATION_MIN);
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //CURRENT MODE Line 5
    uLCD.printf("Mode: %s", getCurrentMode() );
    uLCD.printf("\n\n");

    //COLOR WHEEL COLOR Line 5
    uLCD.printf("CW Color: %s", getCurrentColorWheel() );
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Bottom Line
    uLCD.locate(0,15);
    uLCD.printf(" Back");
    //UPDATE WITH VARS
    
    cursor_y = 3;
    cursor_y = 123;
    page = VIEW_SETTINGS;
    updateCursor();
    line = 0;
}

//page that allows the user to edit all of the different 
void changeSettingsScreen() {
    
    page = CHANGE_SETTINGS;
    line = 0;
    
    // Set up
    uLCD.cls();
    uLCD.color(WHITE);
    uLCD.text_width(1); // size text
    uLCD.text_height(1.25);
    uLCD.locate(0,0);
    uLCD.printf("CHANGE SETTINGS\n\n");
    
    //ALARM TIME Line 1
    uLCD.printf(" Alarm: ");
    //UPDATE WITH VARS
    char alarmTime[32];
    strftime(alarmTime, 32, "%I:%M %p", localtime(&ALARM_TIME));
    uLCD.printf("%s", alarmTime);
    uLCD.printf("\n\n");
    
    //LOCAL TIME Line 2
    uLCD.printf(" Local: ");
    //UPDATE WITH VARS
    char curTime[32];
    strftime(curTime, 32, "%I:%M %p", localtime(&LOCAL_TIME));
    uLCD.printf("%s", curTime);
    uLCD.printf("\n\n");
    
    //SNOOZE DURATION Line 3
    uLCD.printf(" Snooze Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("%d", SNOOZE_DURATION_MIN);
    uLCD.printf("min");
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 4
    uLCD.printf(" Sunset Dur: ");
    //UPDATE WITH VARS
    uLCD.printf("%d", SUNRISE_AND_SUNSET_DURATION_MIN);
    uLCD.printf("min");
    uLCD.printf("\n");

    //SUNSET/SUNRISE DURATION Line 5
    uLCD.printf(" Mode: %s", getCurrentMode() );
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Line 6
    uLCD.printf(" CW Color: %s", getCurrentColorWheel() );
    uLCD.printf("\n\n");

    //SUNSET/SUNRISE DURATION Bottom Line
    uLCD.locate(0,15);
    uLCD.printf(" Back");
    //UPDATE WITH VARS
    
    cursor_x = 4;
    cursor_y = 19;
    cursor_radius = 2;
    cursor_color = RED;
    
    uLCD.filled_circle(cursor_x, cursor_y, cursor_radius, cursor_color);
}

void menuScreen() {
    uLCD.cls();

    uLCD.color(WHITE);
    //VIEW SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    
    uLCD.locate(0,0);
    uLCD.printf("MENU");

    uLCD.locate(1,2);
    uLCD.printf("View Settings");
    //CHANGE SETTINGS
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,5);
    uLCD.printf("Change Settings");
    //BACK
    uLCD.text_width(1); 
    uLCD.text_height(1);
    uLCD.locate(1,15);
    uLCD.printf("Back");
    //CURSOR
    //NEED SOMETHING FOR SELECTING
    
    page = MENU;
    line  = 0;
    updateCursor();
}


void homeScreen(){
    page = MAIN;
    line = 0;
    
    uLCD.cls();
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
    uLCD.printf("%s", getCurrentMode() );//end switch

    updateCursor();
    
    page = MAIN;
    line = 0;

}

void updatingAlarm() {
    //hour, min, am/pm vars
    int hour = 1;
    int min = 0;
    int ampm = 0; // 0 is am 1 is pm
    bool notdone = true;
    
    char bnum = 0;
    //lazy solution
    //do hours then mins then am/pm
    //No ability to go back once confirmed
    
    //set hours
    while(notdone){
        //flash hour
        uLCD.locate(8,2);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(8,2);
        uLCD.printf("%d", hour);
        wait(0.15);
            
        if (upPB == 0 | rightPB == 0) {
            hour++;
            if (hour > 12) hour = 12; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            hour--;
            if (hour < 1) hour = 1; // minimum value
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){          
            if (blue.getc()=='!') {
                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
                    if (blue.getc() == '0') { // push
                    
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            hour++;
                            if (hour > 12) hour = 12; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            hour--;
                            if (hour < 1) hour = 1; // minimum value
                        } else if (bnum == '1') { // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable
    }
    //set min
    notdone = true;
    while(notdone){
        //flash hour
        uLCD.locate(11,2);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(11,2);
        uLCD.printf("%d", min);
        wait(0.15);
            
        if (upPB == 0 | rightPB == 0) {
            min++;
            if (hour > 59) hour = 59; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            min--;
            if (min < 0) min = 0; // minimum value
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            min++;
                            if (hour > 59) hour = 59; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            min--;
                            if (min < 0) min = 0; // minimum value
                        } else if (bnum == '1') { // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    }
    //set am/pm
    notdone = true;
    while(notdone){
        //flash hour
        uLCD.locate(14,2);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(14,2);
        if(ampm == 0) uLCD.printf("AM");
        else uLCD.printf("PM");
        wait(0.15);
            
        if (upPB == 0 || rightPB == 0 || downPB == 0 || leftPB == 0) {
            if(ampm == 0)ampm = 1;
            else ampm = 0;
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
        
                    if (blue.getc() == '0') { // push
                        
                        if (bnum == '5' || bnum == '8' || bnum == '6' || bnum == '7') { // UP & RIGHT & DOWN & LEFT
                            if(ampm == 0)ampm = 1;
                            else ampm = 0;
                        } else if (bnum == '1') {// 1 pressed
                            notdone = false;
                        }
                    } // close if release '0'
                } // close if release 'B'
            }// == !
        } // if readable
    }
    
    ALARM_TIME = hour*3600 + min*60 + ampm*43140;//convert into secs (43140 is seconds from 12am to 11:59am)
    wait(0.9);

}


void updatingLocal() {
    //hour, min, am/pm vars
    int hour = 1;
    int min = 0;
    int ampm = 0; // 0 is am 1 is pm
    bool notdone = true;
    
    char bnum = 0;
    //lazy solution
    //do hours then mins then am/pm
    //No ability to go back once confirmed
    
    //set hours
    while(notdone){
        //flash hour
        uLCD.locate(8,4);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(8,4);
        uLCD.printf("%d", hour);
        wait(0.15);

        if (upPB == 0 | rightPB == 0) {
            hour++;
            if (hour > 12) hour = 12; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            hour--;
            if (hour < 1) hour = 1; // minimum value
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                    
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            hour++;
                            if (hour > 12) hour = 12; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            hour--;
                            if (hour < 1) hour = 1; // minimum value
                        } else if (bnum == '1') { // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    }
    //set min
    notdone = true;
    while(notdone){
        //flash hour
        uLCD.locate(11,4);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(11,4);
        uLCD.printf("%d", min);
        wait(0.15);
            
        if (upPB == 0 | rightPB == 0) {
            min++;
            if (hour > 59) hour = 59; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            min--;
            if (min < 0) min = 0; // minimum value
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            min++;
                            if (hour > 59) hour = 59; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            min--;
                            if (min < 0) min = 0; // minimum value
                        } else if (bnum == '1') { // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    }
    //set am/pm
    notdone = true;
    while(notdone){
        //flash hour
        uLCD.locate(14,4);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(14,4);
        if(ampm == 0) uLCD.printf("AM");
        else uLCD.printf("PM");
        wait(0.15);
            
        if (upPB == 0 || rightPB == 0 || downPB == 0 || leftPB == 0) {
            if(ampm == 0)ampm = 1;
            else ampm = 0;
        } else if (centerPB == 0) {
            notdone = false;
        } else if (blue.readable()){
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        if (bnum == '5' || bnum == '8' || bnum == '6' || bnum == '7') { // UP & RIGHT & DOWN & LEFT
                            if(ampm == 0)ampm = 1;
                            else ampm = 0;
                        } else if (bnum == '1') { // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable
    }
    
    int timeTemp = hour*3600 + min*60 + ampm*43140;//convert into secs (43140 is seconds from 12am to 11:59am)
    set_time(timeTemp);
    wait(0.9);
}


void updatingSnooze() {
        
    bool selected = false;

    char bnum = 0;
    
    while (!selected) {
        
        uLCD.locate(12,6);
        
        uLCD.printf("  ");
        Thread::wait(150);
        uLCD.locate(12,6);
        uLCD.printf("%d", SNOOZE_DURATION_MIN);
        Thread::wait(150);
        
        if (upPB == 0 | rightPB == 0) {
            SNOOZE_DURATION_MIN++;
            if (SNOOZE_DURATION_MIN > 30) SNOOZE_DURATION_MIN = 30; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            SNOOZE_DURATION_MIN--;
            if (SNOOZE_DURATION_MIN < 1) SNOOZE_DURATION_MIN = 1; // minimum value
        } else if (centerPB == 0) {
            selected = true;
        } else if (blue.readable()){
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            SNOOZE_DURATION_MIN++;
                            if (SNOOZE_DURATION_MIN > 30) SNOOZE_DURATION_MIN = 30; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            SNOOZE_DURATION_MIN--;
                            if (SNOOZE_DURATION_MIN < 1) SNOOZE_DURATION_MIN = 1; // minimum value
                        } else if (bnum == '1') { // RIGHT
                            selected = true;
                        }
                    } // close if release
                } // if == B

            }// == !
        } // if readable && == !
    }
    Thread::wait(900);

}

void updatingSun() {
    
    bool selected = false;
    char bnum = 0;
    
    while (!selected) {
        
        uLCD.locate(13,8);
        
        uLCD.printf("  ");
        Thread::wait(150);
        uLCD.locate(13,8);
        uLCD.printf("%d", SUNRISE_AND_SUNSET_DURATION_MIN);
        Thread::wait(150);
        
        if (upPB == 0 | rightPB == 0) {
            SUNRISE_AND_SUNSET_DURATION_MIN++;
            if (SUNRISE_AND_SUNSET_DURATION_MIN > 60) SUNRISE_AND_SUNSET_DURATION_MIN = 60; // maximum value
        } else if (downPB == 0 | leftPB == 0) {
            SUNRISE_AND_SUNSET_DURATION_MIN--;
            if (SUNRISE_AND_SUNSET_DURATION_MIN < 5) SUNRISE_AND_SUNSET_DURATION_MIN = 5; // minimum value
        } else if (centerPB == 0) {
            selected = true;
        } else if (blue.readable()){
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                    
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            SUNRISE_AND_SUNSET_DURATION_MIN++;
                            if (SUNRISE_AND_SUNSET_DURATION_MIN > 60) SUNRISE_AND_SUNSET_DURATION_MIN = 60; // maximum value
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            SUNRISE_AND_SUNSET_DURATION_MIN--;
                            if (SUNRISE_AND_SUNSET_DURATION_MIN < 5) SUNRISE_AND_SUNSET_DURATION_MIN = 5; // minimum value
                        } else if (bnum == '1') { // RIGHT
                            selected = true;
                        }
                    } // close if release
                } // if == B

            }// == !
        } // if readable && == !
    }
    Thread::wait(900);

}

void updatingMode() {
        
    bool selected = false;
    char bnum=0;
    
    while (!selected) {
        //min location
        uLCD.locate(7,10);
            
        //clear line before printing next one
        uLCD.printf("           ");
            
        Thread::wait(150);
        uLCD.locate(7,10);
        uLCD.printf("%s", getCurrentMode());
            
        Thread::wait(150);
        
        if (upPB == 0 | rightPB == 0) {
            CURRENT_MODE++;
            if (CURRENT_MODE > 4) CURRENT_MODE = 0;
        } else if (downPB == 0 | leftPB == 0) {
            CURRENT_MODE--;
            if (CURRENT_MODE < 0) CURRENT_MODE = 4;
        } else if (centerPB == 0) {
            selected = true;
        } else if (blue.readable()){
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                    bnum = blue.getc(); //button number
                    if (blue.getc() == '0') { // push
                        if (bnum == '5' || bnum == '8') { // UP & RIGHT
                            CURRENT_MODE++;
                            if (CURRENT_MODE > 4) CURRENT_MODE = 0;
                        } else if (bnum == '6' || bnum == '7') { // DOWN & LEFT
                            CURRENT_MODE--;
                            if (CURRENT_MODE < 0) CURRENT_MODE = 4;
                        } else if (bnum == '1') { // RIGHT
                            selected = true;
                        }
                    } // close if release
                } // if == B

            }// == !
        } // if readable && == !
    }
    Thread::wait(900);
    
}

void updatingColorWheelColor() {
        
    bool selected = false;
    char bnum=0;
    
    while (!selected) {
        //min location
        uLCD.locate(11,12);
            
        //clear line before printing next one
        uLCD.printf("           ");
            
        Thread::wait(150);
        uLCD.locate(11,12);
        uLCD.printf("%s", getCurrentColorWheel());
            
        Thread::wait(150);
        
        if (upPB == 0 | rightPB == 0) {
            changeColorWheel(1);
        } else if (downPB == 0 | leftPB == 0) {
            changeColorWheel(-1);
        } else if (centerPB == 0) {
            selected = true;
        } else if (blue.readable()){
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                
                    bnum = blue.getc(); //button number
                    if (blue.getc() == '0') { // push
                    
                        if (bnum == '5' || bnum == '8') {
                            changeColorWheel(1);
                        } else if (bnum == '6' || bnum == '7') {
                            changeColorWheel(-1);
                        } else if (bnum == '1') {
                            selected = true;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    }
    Thread::wait(900);
    
}


void selection() {
    
    switch (page) {
        case MAIN:
            menuScreen();
            break;
            
        case MENU:
            if (line == 0) {
                viewSettingsScreen();
            } else if (line == 1) {
                changeSettingsScreen();
            } else {
                homeScreen();
            }
            break;
            
        case VIEW_SETTINGS:
            menuScreen();
            break;
            
        case CHANGE_SETTINGS:
            if (line == 0) {
                updatingAlarm();
            } else if (line == 1) {
                updatingLocal();
            } else if (line == 2) {
                updatingSnooze();
            } else if (line == 3) {
                updatingSun();
            } else if (line == 4) {
                updatingMode();
            } else if (line == 5) { 
                updatingColorWheelColor();
            } else if (line == 6) {
                menuScreen();
            }
            break;
            
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////
///////////// WHERE THE LED CODE GOES //////////////////////////
////////////////////////////////////////////////////////////////

void sunrise() {
 for (int i = 1; i<ring_number+1; i++)
    {
    var.switchLightOff(ring_number);
    var.switchLightOn(ring_number);
    var.changeColor(ring_number, (rgb_color) {20*i,0,0});
    Thread::wait(500);
    }   
}

void lightOn() {
    var.switchLightOff(ring_number);
    var.switchLightOn(ring_number);
    var.changeColor(ring_number, (rgb_color) {255,255,255});
    Thread::wait(500);
}

void lightRainbow() {
    var.switchLightOff(ring_number);
    var.switchLightOn(ring_number);
    var.circleRainbow(ring_number);
    Thread::wait(500);
}

void lightOff() {
    var.changeColor(ring_number, (rgb_color) {0,0,0});
    Thread::wait(500);
}

void lightColorWheel() {
    if (USE_BLUETOOTH) {
        var.changeColor(ring_number, (rgb_color) {BLUETOOTH_RED, BLUETOOTH_GREEN, BLUETOOTH_BLUE});
    } else {
        switch (COLOR_WHEEL_COLOR) {
            case RED:
                var.changeColor(ring_number, (rgb_color) {255, 0, 0});
                break;
            case ORANGE:
                var.changeColor(ring_number, (rgb_color) {255, 150, 0});
                break;
            case YELLOW:
                var.changeColor(ring_number, (rgb_color) {255, 255, 0});
                break;
            case GREEN:
                var.changeColor(ring_number, (rgb_color) {0, 255, 0});
                break;
            case BLUE:
                var.changeColor(ring_number, (rgb_color) {0, 0, 255});
                break;
            case PURPLE:
                var.changeColor(ring_number, (rgb_color) {120, 0, 120});
                break;
            case PINK:
                var.changeColor(ring_number, (rgb_color) {255, 20, 147});
                break;
            case BLUETOOTH:
                var.changeColor(ring_number, (rgb_color) {BLUETOOTH_RED, BLUETOOTH_GREEN, BLUETOOTH_BLUE});
                break;
            default:
                var.changeColor(ring_number, (rgb_color) {255, 255, 255});
                break;
        }
    }
    Thread::wait(500);
}

void playAlarmSound(){
    while(1){
        speaker.period(1.0/150.0);
        speaker=0.1;
        Thread::wait(1000); 
        speaker=0;
        Thread::wait(1000);
    }
}

void startSunrise(){
    /*
    //Just some sudo code to get started can do more with LEDs hooked up
    while(1){
        //as the local time gets closer to the alarm time it gets brighter
        LED = (LOCAL_TIME%86400) / ALARM_TIME;
        Thread::wait(1000);//increment every 1 second
    }
    */
}

void beginSunrise() {
    if ( /** IF INSIDE SUNRISE TIME **/) {
        bool skipped = false;
        if (int i = 0; i < /** IF INSIDE SUNRISE TIME **/; i++) {
            if (i/ (maxTime/3) )
        }

    }
}

void led_states() { // thread for all the LED Code 
    while (1) {
        int value = CURRENT_MODE;
        switch (value) {
            case SLEEP: // sunset code and "go off" at alarm time and every snooze duration
                led1 = led4 = 1;
                led3 = led4 = 0;
                beginSunrise();
                break;
            case COLOR_WHEEL: //usse color from COLOR_WHEEL_COLOR or BLUETOOTH (check the USE_BLUETOOTH variable)
                led1 = led3 = 1;
                led2 = led4 = 0;
                lightColorWheel();
                break;
            case RAINBOW: // Run through the rainbow
                led2 = led4 = 1;
                led1 = led3 = 0.5;
                lightRainbow();
                break;
            case LIGHT_ON: // COLOR = WHITE
                led1 = led2 = led3 = led4 = 1;
                lightOn();
                break;
            case LIGHT_OFF: // LEDS OFF
                led1 = led2 = led3 = led4 = 0;
                lightOff();
                break;
            default:
                break;
        }
    }
}

int main() {
    // ahh
    uLCD.cls();
    uLCD.baudrate(BAUD_3000000); //jack up baud rate to max for fast display

    set_time(0);                //set the time to 0 or 12:00am to begin

    snoozePB.mode(PullUp);
    sleepPB.mode(PullUp);
    upPB.mode(PullUp);
    downPB.mode(PullUp);
    leftPB.mode(PullUp);
    rightPB.mode(PullUp);
    centerPB.mode(PullUp);

    Thread::wait(1.0);
    homeScreen();
    
    char bnum=0;

    thread.start(led_states);
    int counter = 0;
    
    //ADD TO MERGE
    int snoozeCount = 0;

    while(1) {
        LOCAL_TIME = time(NULL);            //update local time
        pc.printf("hello\n\r");

        if (downPB == 0) {
            line++;
            updateCursor();
            Thread::wait(500);
        } else if (upPB == 0) {
            line--;
            updateCursor();
            Thread::wait(500);
        } else if (centerPB == 0) {
            selection();
        } else if (blue.readable()){
            if (blue.getc()=='!') {
                bnum = blue.getc();
            
                if (bnum == 'B') { //button data
                    bnum = blue.getc(); //button number
                    if (blue.getc() == '0') { // push
                        if (bnum == '5') { // UP
                            line--;
                            updateCursor();
                            Thread::wait(500);
                        } else if (bnum == '6') { // DOWN
                            line++;
                            updateCursor();
                            Thread::wait(500);
                        } else if (bnum == '1') { // RIGHT
                            selection();
                        }
                    } 
                } else if (bnum == 'C'){// == !
                    pc.printf("C\n\r");
                    int i = 0;
                    char value = 0;
                    while (blue.readable()) {
                        pc.printf("i: %d", i);
                        value = blue.getc();
                        pc.printf("%d", value);
                        if (i == 0) {
                            BLUETOOTH_RED = value;
                        } else if (i == 1) {
                            BLUETOOTH_GREEN = value;
                        } else if (i == 2) {
                            BLUETOOTH_BLUE = value;
                        }
                        i++;
                    }
                    pc.printf("\n\r");
                    pc.printf("Summary R: %d, G: %d, B: %d, W: \n\r", BLUETOOTH_RED, BLUETOOTH_GREEN, BLUETOOTH_BLUE); // This shows the value in base 10
                }
            } 
        } // if readable

        Thread::wait(50); // 50 * 20 * 60 = 60 seconds
        // rewrite the time every 60 seconds
        if (counter == 20 * 60) {
            counter = 0;
            if (page == MAIN) {
                uLCD.text_width(2.5); 
                uLCD.text_height(2.5);
                uLCD.locate(0.5,2.5);
                //Pull current time and set it to a charArray
                char curTime[32];
                strftime(curTime, 32, "%I:%M %p\n", localtime(&LOCAL_TIME));
                uLCD.printf("%s", curTime);
            }
        }
        counter++;s
        
        //ADD TO BRANCH
        //checking if alarm should start
        //WANT TO START EARLIER FOR SUNRISE
        //testing
        //pc.printf("Local/Alarm check\r\n");
        //pc.printf("Local: %lld\r\n", (long long) LOCAL_TIME);
        //pc.printf("Alarm: %lld\r\n", (long long) ALARM_TIME);
        
        
        if(LOCAL_TIME%86400 + SUNRISE_AND_SUNSET_DURATION_MIN * 60 > ALARM_TIME && LOCAL_TIME%86400 + SUNRISE_AND_SUNSET_DURATION_MIN * 60 < (ALARM_TIME + 2)){//86400 seconds in 24 hour
            
            pc.printf("doing the alarm\r\n");
           //START SUNRISE FUNCTION CALL
           sunriseThread.start(startSunrise);
           //END SUNRISE FUNCTION CALL         
           //find good way to term the thread 
        }//end if
        
        //start sound alarm
        //add snooze duration to alarm time determined by how many times the snoozeCount has been incremented
        if(LOCAL_TIME%86400 > ALARM_TIME + (SNOOZE_DURATION_MIN * 60 * snoozeCount) && LOCAL_TIME%86400 < (ALARM_TIME + 2)  + (SNOOZE_DURATION_MIN * 60 * snoozeCount)){//86400 seconds in 24 hour
             speakerThread.start(playAlarmSound);
             bool notdone = true;
             //play alarm until done
             while(notdone){
                 if (blue.readable()){          
                    if (blue.getc()=='!') {
        
                        if (blue.getc()=='B') { //button data
                            
                            bnum = blue.getc(); //button number
             
                            if (blue.getc() == '0') { // push
                                //if button 3 is hit stop the alarm and leave the loop
                                if (bnum == '3') {//sleep button
                                    speakerThread.terminate();
                                    Thread::wait(1000);
                                    notdone = false;
                                    speaker = 0;
                                    //reset snoozeCount to 0 so the alarm will go off at the right time again
                                    snoozeCount = 0;
                                }
                                else if (bnum == '2'){//snooze
                                    speakerThread.terminate();
                                    Thread::wait(1000);
                                    speaker = 0;
                                    //increment the snoozeCounter in for computation in if statement
                                    snoozeCount++;
                                    notdone = false;              
                                }
                            }
                        }
                    }
                }
             }//end while
        }//end if

    } // while loop

}
