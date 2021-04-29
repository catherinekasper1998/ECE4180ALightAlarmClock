#include "mbed.h"
#include "uLCD_4DGL.h"
#include "rtos.h"

/*
Final Project for 4180.
A light alarm clock with custom sunrise/sunset settings
*/
//LCD screen
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;
Serial pc(USBTX, USBRX); // tx, rx
Serial blue(p28,p27);
Thread thread;

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
time_t ALARM_TIME;
int SNOOZE_DURATION_MIN = 5;
int SUNRISE_AND_SUNSET_DURATION_MIN = 30;
volatile int CURRENT_MODE = SLEEP;
int RAINBOW_COLOR = WHITE;
volatile int COLOR_WHEEL_COLOR = RED; // this is a state variable 
volatile bool USE_BLUETOOTH = false;
volatile int BLUETOOTH_COLOR = WHITE; // This is what bluetooth color wheel color should be set yo

int cursor_x = 4;
int cursor_y = 19;
int cursor_radius = 2;
int cursor_color = RED;

int charToInt(char character) {
    pc.printf("Char to int input %c\n", character);
    switch (character) {
        case '0':
            pc.printf("output %d\n", 0);
            return 0;
        case '1':
            pc.printf("output %d\n", 1);
            return 1;
        case '2':
            pc.printf("output %d\n", 2);
            return 2;
        case '3':
            pc.printf("output %d\n", 3);
            return 3;
        case '4':
            pc.printf("output %d\n", 4);
            return 4;
        case '5':
            pc.printf("output %d\n", 5);
            return 5;
        case '6':
            pc.printf("output %d\n", 6);
            return 6;
        case '7':
            pc.printf("output %d\n", 7);
            return 7;
        case '8':
            pc.printf("output %d\n", 8);
            return 8;
        case '9':
            pc.printf("output %d\n", 9);
            return 9;
        default:
            pc.printf("ERROR IN CHAR TO INT");
            return -1;
    }
}

int getIntHr(char* time) {
    return (charToInt(time[0]) * 10) + charToInt(time[1]);
}

int getIntMin(char* time) {
    return (charToInt(time[0]) * 10) + charToInt(time[1]);
}

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
            pc.printf("in main\n");
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
            pc.printf("in view settings");
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

//CAN PROBABLY BE DELETED
void editVariable(){
    //if statements are just to test certain lines
    //ALARM TIME LOCATION
    if(0){
        //hour location
        uLCD.locate(8,2);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(8,2);
        uLCD.printf("12");
        Thread::wait(500);
        
        //min location
        uLCD.locate(11,2);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(11,2);
        uLCD.printf("59");
        Thread::wait(500);
        
        //am/pm
        uLCD.locate(13,2);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(13,2);
        uLCD.printf("am");
        Thread::wait(500);
    }
    //LOCAL TIME LOCATION
    if (0) {
        //hour location
        uLCD.locate(8,4);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(8,4);
        uLCD.printf("12");
        Thread::wait(500);
        
        //min location
        uLCD.locate(11,4);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(11,4);
        uLCD.printf("59");
        Thread::wait(500);
        
        //am/pm
        uLCD.locate(13,4);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(13,4);
        uLCD.printf("am");
        Thread::wait(500);
    }
    //SNOOZE DUR LOCATION
    if(0){
        //min location
        uLCD.locate(13,6);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(13,6);
        uLCD.printf("15");
        Thread::wait(500);
    }
    //SUNSET DUR LOCATION
    if(0){
        //min location
        uLCD.locate(13,8);
        
        uLCD.printf("  ");
        Thread::wait(500);
        uLCD.locate(13,8);
        uLCD.printf("30");
        Thread::wait(500);
    }
    //MODE LOCATION
    if(1) {
        
        for(int modeInt = 0; modeInt < 5; modeInt++){
            //min location
            uLCD.locate(7,10);
            
            //clear line before printing next one
            uLCD.printf("           ");
            
            Thread::wait(1000);
            uLCD.locate(7,10);
            switch(modeInt){
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
            }//end switch
            Thread::wait(1000);
        }//end for
    }//end if
}
//END OF PROBABLY CAN BE DELETED

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
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            hour++;
                            if (hour > 12) hour = 12; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            hour--;
                            if (hour < 1) hour = 1; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
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
        uLCD.locate(11,2);   
        uLCD.printf("  ");
        wait(0.15);
        uLCD.locate(11,2);
        uLCD.printf("%d", min);
        wait(0.15);
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            min++;
                            if (hour > 59) hour = 59; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            min--;
                            if (min < 0) min = 0; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
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
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8' || bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT & DOWN & LEFT
                            if(ampm == 0)ampm = 1;
                            else ampm = 0;
                        } 
                        else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
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
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            hour++;
                            if (hour > 12) hour = 12; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            hour--;
                            if (hour < 1) hour = 1; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
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
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            min++;
                            if (hour > 59) hour = 59; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            min--;
                            if (min < 0) min = 0; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
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
            
        if (blue.readable()){
            pc.printf("READABLE Serial\n");
          
            if (blue.getc()=='!') {

                if (blue.getc()=='B') { //button data
                    
                    bnum = blue.getc(); //button number
     
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8' || bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT & DOWN & LEFT
                            if(ampm == 0)ampm = 1;
                            else ampm = 0;
                        } 
                        else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // 1 pressed
                            notdone = false;
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    }
    
    int timeTemp = hour*3600 + min*60 + ampm*43140;//convert into secs (43140 is seconds from 12am to 11:59am)
    set_time(timeTemp);
    wait(0.9);
    /*
    uLCD.locate(8,4);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(8,4);
    uLCD.printf("12");
    wait(0.5);
        
    //min location
    uLCD.locate(11,4);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(11,4);
    uLCD.printf("59");
    wait(0.5);
        
    //am/pm
    uLCD.locate(14,4);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(14,4);
    uLCD.printf("am");
    wait(0.5);
    */
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
            pc.printf("READABLE Serial\n");
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            SNOOZE_DURATION_MIN++;
                            if (SNOOZE_DURATION_MIN > 30) SNOOZE_DURATION_MIN = 30; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            SNOOZE_DURATION_MIN--;
                            if (SNOOZE_DURATION_MIN < 1) SNOOZE_DURATION_MIN = 1; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // RIGHT
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
            pc.printf("READABLE Serial\n");
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            SUNRISE_AND_SUNSET_DURATION_MIN++;
                            if (SUNRISE_AND_SUNSET_DURATION_MIN > 60) SUNRISE_AND_SUNSET_DURATION_MIN = 60; // maximum value
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            SUNRISE_AND_SUNSET_DURATION_MIN--;
                            if (SUNRISE_AND_SUNSET_DURATION_MIN < 5) SUNRISE_AND_SUNSET_DURATION_MIN = 5; // minimum value
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // RIGHT
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
            pc.printf("READABLE Serial\n");
            if (blue.getc()=='!') {
            
                if (blue.getc()=='B') { //button data
                
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");
                    
                        if (bnum == '5' || bnum == '8') {
                            pc.printf("BUTTON 5 or 8\n"); // UP & RIGHT
                            CURRENT_MODE++;
                            if (CURRENT_MODE > 4) CURRENT_MODE = 0;
                        } else if (bnum == '6' || bnum == '7') {
                            pc.printf("BUTTON 6 or 7\n"); // DOWN & LEFT
                            CURRENT_MODE--;
                            if (CURRENT_MODE < 0) CURRENT_MODE = 4;
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // RIGHT
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
                            pc.printf("BUTTON No. 1"); // RIGHT
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
            pc.printf("MAIN to MENU \n");
            menuScreen();
            break;
            
        case MENU:
            pc.printf("MENU to ");
            if (line == 0) {
                pc.printf("VIEW SETTINGS\n");
                viewSettingsScreen();
            } else if (line == 1) {
                pc.printf("CHANGE SETTINGS\n");
                changeSettingsScreen();
            } else {
                pc.printf("MAIN\n");
                homeScreen();
            }
            break;
            
        case VIEW_SETTINGS:
            pc.printf("VIEW SETTINGS to MENU");
            menuScreen();
            break;
            
        case CHANGE_SETTINGS:
            pc.printf("CHANGE SETTINGS to ");
            if (line == 0) {
                pc.printf("ALARM TIME\n");
                updatingAlarm();
            } else if (line == 1) {
                pc.printf("LOCAL TIME \n");
                updatingLocal();
            } else if (line == 2) {
                pc.printf("SNOOZE DUR \n");
                updatingSnooze();
            } else if (line == 3) {
                pc.printf("SUN DUR \n");
                updatingSun();
            } else if (line == 4) {
                pc.printf("Mode \n");
                updatingMode();
            } else if (line == 5) { 
                pc.printf("COLOR WHEEL COLOR \n");
                updatingColorWheelColor();
            } else if (line == 6) {
                pc.printf("BACK & SAVE \n");
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

void led_states() { // thread for all the LED Code 
    while (1) {
        int value = CURRENT_MODE;
        switch (value) {
            case SLEEP: // sunset code and "go off" at alarm time and every snooze duration
                led1 = led4 = 1;
                led3 = led4 = 0;
                break;
            case COLOR_WHEEL: //usse color from COLOR_WHEEL_COLOR or BLUETOOTH (check the USE_BLUETOOTH variable)
                led1 = led3 = 1;
                led2 = led4 = 0;
                bool enter = false;
                if (USE_BLUETOOTH){
                    enter = true;
                    while (blue.readable()){
                        pc.printf("%d", blue.getc());
                    }
                } else if (enter) {
                    pc.printf("\n");
                }
                break;
            case RAINBOW: // Run through the rainbow
                led2 = led4 = 1;
                led1 = led3 = 0.5;
                break;
            case LIGHT_ON: // COLOR = WHITE
                led1 = led2 = led3 = led4 = 1;
                break;
            case LIGHT_OFF: // LEDS OFF
                led1 = led2 = led3 = led4 = 0;
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
    
    while(1) {
        LOCAL_TIME = time(NULL);            //update local time

        //pc.printf("LOOPING\n");
        if (downPB == 0) {
            pc.printf("Down\n");
            line++;
            updateCursor();
            Thread::wait(500);
        } else if (upPB == 0) {
            pc.printf("Up\n");
            line--;
            updateCursor();
            Thread::wait(500);
        } else if (centerPB == 0) {
            pc.printf("Center\n");
            selection();
            
        } else if (blue.readable()){
            pc.printf("READABLE Serial\n");
            if (blue.getc()=='!') {

                bnum = blue.getc();
            
                if (bnum == 'B') { //button data
                
                    bnum = blue.getc(); //button number
                    
                    if (blue.getc() == '0') { // push
                        pc.printf("PUSH\n");

                        if (bnum == '5') {
                            pc.printf("BUTTON 5\n"); // UP
                            line--;
                            updateCursor();
                            Thread::wait(500);
                        } else if (bnum == '6') {
                            pc.printf("BUTTON 6\n"); // DOWN
                            line++;
                            updateCursor();
                            Thread::wait(500);
                        } else if (bnum == '7') {
                            pc.printf("BUTTON 7"); // LEFT
                        } else if (bnum == '8') {
                            pc.printf("BUTTON 8"); // RIGHT
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // RIGHT
                            selection();
                        }
                    } else {// close if release
                        pc.printf("NOT 0\n");
                    }
                } else if (bnum == 'C'){// == !
                    pc.printf("C\n");
                    int i = 0;
                    int r, g, b, w = 0;
                    char value = 0;
                    while (blue.readable()) {
                        pc.printf("i: %d", i);
                        value = blue.getc();
                        pc.printf("%d", value);
                        if (i == 0) {
                            r = value;
                        } else if (i == 1) {
                            g = value;
                        } else if (i == 2) {
                            b = value;
                        } else if (i == 3) {
                            w = value;
                        }
                        i++;
                    }
                    pc.printf("\n");
                    pc.printf("Summary R: %d, B: %d, G: %d, W: %d", r, g, b, w); // This shows the value in base 10
                }
            } else { // if NOT !
                pc.printf("Not !!\n");
            }
        } // if readable
    } // while loop

                        if (bnum == '5') {
                            pc.printf("BUTTON 5\n"); // UP
                            line--;
                            updateCursor();
                            wait_ms(500);
                        } else if (bnum == '6') {
                            pc.printf("BUTTON 6\n"); // DOWN
                            line++;
                            updateCursor();
                            wait_ms(500);
                        } else if (bnum == '7') {
                            pc.printf("BUTTON 7"); // LEFT
                        } else if (bnum == '8') {
                            pc.printf("BUTTON 8"); // RIGHT
                        } else if (bnum == '1') {
                            pc.printf("BUTTON No. 1"); // RIGHT
                            selection();
                        }
                    } // close if release
                } // if == B
            }// == !
        } // if readable && == !
    } // while loop

}
