#include "mbed.h"

//Blink rate in milliseconds
#define BLINKING_RATE     500ms

int main()
{
    //Initialise the digital pin LED1 as an output
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);

    while (true) {
        led2 = !led2;
        led3 = !led3;
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
