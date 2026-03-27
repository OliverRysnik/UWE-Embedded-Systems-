#include "mbed.h"
#include "arm_book_lib.h"

//List of 6 input pins (D2 through D7) as buttons
DigitalIn buttons[] = {
    DigitalIn(D2), DigitalIn(D3), DigitalIn(D4), 
    DigitalIn(D5), DigitalIn(D6), DigitalIn(D7)
};

//Scans all buttons to see if any are pressed.
int read_button() {
    for (int i = 0; i < 6; i++) {
        if (buttons[i]) { 
            return i;     
        }
    }
    return -1; 
}

//The password
int password[4] = {1, 1, 1, 1};
//Admin code for override
int admin_code[4] = {5, 5, 5, 5};


int main() {
    //LED's added
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    
    //Make sure LEDs start as OFF
    led1 = OFF;
    led2 = OFF;
    led3 = OFF;

    //counter for failure
    int failed_attempts = 0;
    //Flag to track if system locked
    bool is_locked = false;
    //Timer for non-blocking lockdown
    Timer lockdown_timer;

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
                // If in lockdown, check if 60 seconds have passed to auto-unlock
                if (is_locked && chrono::duration_cast<chrono::seconds>(lockdown_timer.elapsed_time()).count() >= 60) {
                    is_locked = false;
                    lockdown_timer.stop();
                    led1 = OFF;
                    led2 = OFF;
                    failed_attempts = 0;
                }

                //Blink LED2 visually while waiting for buttons during lockdown
                if (is_locked || failed_attempts >= 4) {
                    led2 = !led2;
                    ThisThread::sleep_for(100ms); 
                }

                value = read_button();
            } while (value == -1);

            entered_code[i] = value;
            ThisThread::sleep_for(300ms);
        }

        //Check if entered code matches admin code
        bool admin_correct = true;
        for (int i = 0; i < 4; i++) {
            if (entered_code[i] != admin_code[i]) {
                admin_correct = false;
                break;
            }
        }

        //Logic in lockdown mode
        if (is_locked) {
            if (admin_correct) {
                is_locked = false; //Reset state
                lockdown_timer.stop(); // Stop the timer override
                failed_attempts = 0; //Reset counter
                led1 = OFF; //Turn off lockdown indicators
                led2 = OFF;
            }
            //After admin check, ignore entry and restart loop
            continue; 
        }

        //Compare the entered password against the stored password
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            if (entered_code[i] != password[i]) {
                correct = false; 
                break;
            }
        }

        //Output
        if (correct) {
            led1 = ON; //Turn on Good LED
            failed_attempts = 0; //Reset counter on success
            ThisThread::sleep_for(2s);
            led1 = OFF;
        } else {
            led3 = ON; //Turn on Bad LED
            failed_attempts++; //Increase the failure counter 
            ThisThread::sleep_for(1s);
            led3 = OFF;

            //Warning is triggered if 3 incorrect codes are entered consecutively 
            if (failed_attempts == 3) {
                for (int i = 0; i < 30; i++) {
                    led2 = ON;
                    ThisThread::sleep_for(500ms);
                    led2 = OFF;
                    ThisThread::sleep_for(500ms);
                }
            } 
            //Lockdown triggered on 4th incorrect entry
            else if (failed_attempts >= 4) {
                is_locked = true; //Set system to lockdown state
                led1 = ON; //Continuous LED for lockdown
                lockdown_timer.reset(); //Start the 1-minute timer
                lockdown_timer.start();
            }
        }
    }
}
