#include "mbed.h"
#include "uLCD_4DGL.h"

/*
Final Project for 4180.
A light alarm clock with custom sunrise/sunset settings
*/
//LCD screen
uLCD_4DGL uLCD(p9,p10,p11); // serial tx, serial rx, reset pin;
Serial pc(USBTX, USBRX); // tx, rx

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

void updateCursor(){ 
    // UPDATE THE PAGE AND INDEX BEFORE CALLING THIS FUNCTION
    // This function does handle line being "out of bounds" 
    // and reassigns a "wrapped around value" its proper line
    pc.printf(" Circle Location (x: %d , y: %d)\n" , cursor_x, cursor_y);

    uLCD.filled_circle(cursor_x, cursor_y, cursor_radius, BLACK);

    switch (page) {

        case MAIN:              // line should ever only be 0
            pc.printf("in main\n");
            wait_ms(500);      // flash the circle so people know that it is active
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
            } else if (line == 5 | line < 0) {     // Back & Save
                cursor_x = 4;
                cursor_y = 122;
                line = 5;
            }
            break;
        
        case VIEW_SETTINGS: // SHOULD ONLY EVER BE 0
            pc.printf("in view settings");
            wait_ms(500);
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

    //SUNSET/SUNRISE DURATION Line 5
    uLCD.printf("Mode: %s", getCurrentMode() );
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

void editVariable(){
    //if statements are just to test certain lines
    //ALARM TIME LOCATION
    if(0){
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
    //LOCAL TIME LOCATION
    if (0) {
        //hour location
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
        uLCD.locate(13,4);
        
        uLCD.printf("  ");
        wait(0.5);
        uLCD.locate(13,4);
        uLCD.printf("am");
        wait(0.5);
    }
    //SNOOZE DUR LOCATION
    if(0){
        //min location
        uLCD.locate(13,6);
        
        uLCD.printf("  ");
        wait(0.5);
        uLCD.locate(13,6);
        uLCD.printf("15");
        wait(0.5);
    }
    //SUNSET DUR LOCATION
    if(0){
        //min location
        uLCD.locate(13,8);
        
        uLCD.printf("  ");
        wait(0.5);
        uLCD.locate(13,8);
        uLCD.printf("30");
        wait(0.5);
    }
    //MODE LOCATION
    if(1) {
        
        for(int modeInt = 0; modeInt < 5; modeInt++){
            //min location
            uLCD.locate(7,10);
            
            //clear line before printing next one
            uLCD.printf("           ");
            
            wait(1);
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
            wait(1);
        }//end for
    }//end if
}

void updatingAlarm() {
    // the flashing function
    uLCD.locate(8,2);
    
    int currentLocation = 0;
    bool notDone = true;

    char alarmTime[32];
    strftime(alarmTime, 32, "%I:%M %p", localtime(&ALARM_TIME));

    int hr_var = ;
    int min_var = ;
    int am_var = 0;

    while (notDone) {
        
        switch (currentLocation) {
            case 0:
                uLCD.printf("  ");
                wait(0.4);

                uLCD.locate(8,2);

                char alarmTime[32];
                strftime(alarmTime, 32, "%I:%M %p", localtime(&ALARM_TIME));

                uLCD.printf("%d", );
                wait(0.4);

                if (upPB == 0) {
                    // increase value
                } else if (downPB == 0) {
                    // decrease value
                } else if (leftPB == 0) {
                    // move to AM
                } else if (rightPB == 0) {
                    // move to min
                } else if (centerPB == 0) {
                    // save value -> exit update Alarm
                    notDone = false;
                }

                break;
            
            case 1:
                //min location
                uLCD.locate(11,2);
                    
                uLCD.printf("  ");
                wait(0.3);
                uLCD.locate(11,2);
                uLCD.printf("59");
                wait(0.3);
                break;

            case 2:  
                //am/pm
                uLCD.locate(13,2);
                
                uLCD.printf("  ");
                wait(0.3);
                uLCD.locate(13,2);
                uLCD.printf("am");
                wait(0.3);
                break;
        }
    }

    // restablish the page just in case
    int saveCursorLine = line;
    changeSettingsScreen();
    line = saveCursorLine;
    updateCursor();

}

void updatingLocal() {
    //hour location
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
    uLCD.locate(13,4);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(13,4);
    uLCD.printf("am");
    wait(0.5);

}

void updatingSnooze() {
    uLCD.locate(13,6);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(13,6);
    uLCD.printf("15");
    wait(0.5);

}

void updatingSun() {
    uLCD.locate(13,8);
        
    uLCD.printf("  ");
    wait(0.5);
    uLCD.locate(13,8);
    uLCD.printf("30");
    wait(0.5);

}

void updatingMode() {
        for(int modeInt = 0; modeInt < 5; modeInt++){
            //min location
            uLCD.locate(7,10);
            
            //clear line before printing next one
            uLCD.printf("           ");
            
            wait(1);
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
            wait(1);
        }//end for
    
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
                pc.printf("BACK & SAVE \n");
                menuScreen();
            }
            break;
            
        default:
            break;
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

    wait(1.0);
    homeScreen();
    
    while(1) {
        LOCAL_TIME = time(NULL);            //update local time

        //pc.printf("page: %d line: %d \n", page, line);
        if (downPB == 0) {
            pc.printf("Down\n");
            pc.printf("page: %d line: %d \n", page, line);
            line++;
            updateCursor();
            wait_ms(500);
        } else if (upPB == 0) {
            pc.printf("Up\n");
            pc.printf("page: %d line: %d \n", page, line);
            line--;
            updateCursor();
            wait_ms(500);
        } else if (centerPB == 0) {
            pc.printf("Center\n");
            pc.printf("Page : %d line: %d\n", page, line);
            selection();
        }
    }

}

