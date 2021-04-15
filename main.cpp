#include "mbed.h"

/*
Final Project for 4180.
A light alarm clock with custom sunrise/sunset settings

*/

DigitalOut myled(LED1);

int main() {
    while(1) {
        myled = 1;
        wait(0.2);
        myled = 0;
        wait(0.2);
    }
}
