#include "mbed.h"

int main()
{
    //setting up 6 buttons as a list
    DigitalIn buttons[] = { DigitalIn(D2), DigitalIn(D3), DigitalIn(D4), 
                            DigitalIn(D5), DigitalIn(D6), DigitalIn(D7)
    };

    DigitalOut ledCorrect(LED1);   //green light for correct password
    DigitalOut ledWarning(LED2);   //red light for wrong passwod
    DigitalOut ledLockdown(LED3);  //extra indicator for when system is frozen

    int password[] = {1,2,3,4};       //correct code
    int adminPassword[] = {0,0,0,0};  //backup code
    int enteredCode[4];               //store the 4 digits
    int failedAttempts = 0;           //counter to track how many times the user got it wrong
    int index = 0;                    //tracks which digit user is typing

    //infinite loop
    while (true) {
        
        //check every button one by one
        for (int i = 0; i < 6; i++) {
            if (buttons[i].read()) { //if button i pressed
                
                enteredCode[index] = i; //save that number
                index++;                //move to the next slot

                //buffering
                while (buttons[i].read());
                thread_sleep_for(100);  //small pause

                //checks if the 4 digits are right
                if (index == 4) {
                    bool match = true; //assuming they got it right
                    
                    //compare each digit against the real password
                    for (int j = 0; j < 4; j++) {
                        if (enteredCode[j] != password[j]) match = false; //user messed up
                    }

                    if (match) {
                        //correct so turn on the green light 5s
                        ledCorrect = 1;
                        failedAttempts = 0; //reset the counter back to zero
                        thread_sleep_for(5000);
                        ledCorrect = 0;
                    } else {
                        //incorrrect so red light 5s and count the mistake
                        failedAttempts++;
                        ledWarning = 1;
                        thread_sleep_for(5000);
                        ledWarning = 0;
                    }
                    index = 0; //clear the index
                }
            }
        }

        //if they get it wrong 3 times in a row
        if (failedAttempts == 3) {
            //30 seconds of blinking
            for (int blink = 0; blink < 30; blink++) { 
                ledWarning = !ledWarning; //light ON
                thread_sleep_for(500);
                ledWarning = !ledWarning; //light OFF
                thread_sleep_for(500);
            }
            
            ledWarning = 0; //light is off when finished
        }
    }
}
