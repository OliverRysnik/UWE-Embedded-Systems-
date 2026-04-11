#include "mbed.h"

//Blink rate in milliseconds
#define BLINKING_RATE     500ms

int main()
{
    //Initialise the digital pin LED1 as an output
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    
    //keep track of the rate
    int rate = 0;

    while (true) {
        //led2 blinks at 500ms so keep it the same
        led2 = !led2;
        
        //led1 blinks every 1 sec, so double the rate
        if (rate % 2 == 0){
            led1 = !led1;
        }
        //led3 blinks every 2 sec, so quad the rate 
        if (rate % 4 == 0){
            led3 = !led3;
        }

        ThisThread::sleep_for(BLINKING_RATE);
        
        //Adds 1 to the counter
        rate++; 
        
        //makes the rate loop repeat after 4 counts have passed
        if (rate >= 4){
            rate = 0;
        }
    }
}
