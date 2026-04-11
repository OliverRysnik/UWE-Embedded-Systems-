#include "mbed.h"

//Blink rate in milliseconds
#define BLINKING_RATE     200ms

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    
    led1 = 0;
    led2 = 0;
    led3 = 0;

    while (true) {

        led1 = 1;
        ThisThread::sleep_for(BLINKING_RATE);
        led1 = 0;
        ThisThread::sleep_for(BLINKING_RATE);

        led2 = 1;
        ThisThread::sleep_for(BLINKING_RATE);
        led2 = 0;
        ThisThread::sleep_for(BLINKING_RATE);

        led3 = 1;
        ThisThread::sleep_for(BLINKING_RATE);
        led3 = 0;
        ThisThread::sleep_for(BLINKING_RATE);

        led2 = 1;
        ThisThread::sleep_for(BLINKING_RATE);
        led2 = 0;
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
