#include "mbed.h"

//Blink rate in milliseconds
#define BLINKING_RATE     200ms

int main()
{
    //Initialise the digital pin LED1 as an output
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);

    for(int i = 0; i < 5; i++){
        led1 = 1;
        led2 = 1;
        led3 = 1;
        ThisThread::sleep_for(BLINKING_RATE);

        led1 = 0;
        led2 = 0;
        led3 = 0;
        ThisThread::sleep_for(BLINKING_RATE);
    }

    while (true) {
        led1 = 1;
        led2 = 0;
        led3 = 0;
    }
}
