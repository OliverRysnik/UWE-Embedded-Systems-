#include "mbed.h"
#include "arm_book_lib.h"

//List 6 input pins as buttons
DigitalIn buttons[] = {
    DigitalIn(D2), DigitalIn(D3), DigitalIn(D4), 
    DigitalIn(D5), DigitalIn(D6), DigitalIn(D7)
};

//Scans buttons to see if any are pressed
//Returns the index of first button detected as pressed
int read_button() {
    for (int i = 0; i < 6; i++) {
        if (buttons[i]) {
            return i;
        }
    }
    return -1;
}

int password[4] = {1, 1, 1, 1};

int main() {
    
    //LED's added
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    
    //Make sure LEDs start as OFF
    led1 = OFF;
    led3 = OFF;

    //The sequence entered by the user
    int entered_code[4];
    for (int i = 0; i < 6; i++) {
        buttons[i].mode(PullDown);
    }

    while (true) {

        //This gets 4 button presses from the user
        for (int i = 0; i < 4; i++) {
            int value;
            do {
                value = read_button();
            } while (value == -1);

            //Store the ID of button
            entered_code[i] = value;
            
            //Debounce/delay to prevent a press being counted multiple times
            ThisThread::sleep_for(300ms);
        }

        //Compare the entered password against the stored password
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            if (entered_code[i] != password[i]) {
                correct = false;
                break;
            }
        }

        if (correct) {
            led1 = ON; //Turn on success LED
        } else {
            led3 = ON; //Turn on failure LED
        }
        break; 
    }
}
