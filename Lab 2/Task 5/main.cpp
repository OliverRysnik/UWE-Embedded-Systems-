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

//Event log counter for lockdowns
int lockdown_count = 0;

//The display lockdown_count in binary woth LEDs
void display_event_log(int count, DigitalOut& l1, DigitalOut& l2, DigitalOut& l3) {
    l1 = (count & 0x01) ? ON : OFF;
    l2 = (count & 0x02) ? ON : OFF;
    l3 = (count & 0x04) ? ON : OFF;
}

int main() {
    //LED's added
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    
    //Make sure LEDs start as OFF
    led1 = OFF;
    led2 = OFF;
    led3 = OFF;

    int failed_attempts = 0;
    bool is_locked = false;
    Timer lockdown_timer;

    int entered_code[4];
    for (int i = 0; i < 6; i++) {
        buttons[i].mode(PullDown);
    }

    while (true) {
        //Event log display (binary) on LEDs while idle
        if (!is_locked && failed_attempts < 3) {
            display_event_log(lockdown_count, led1, led2, led3);
        }

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

                //Specific lockdown LED behavior (one solid, one blinking) ---
                if (is_locked) {
                    led1 = ON;   //Lockdown LED stays on continuously
                    led2 = !led2; //Second LED blinks intermittently
                    ThisThread::sleep_for(200ms); 
                }

                value = read_button();
            } while (value == -1);

            entered_code[i] = value;
            ThisThread::sleep_for(300ms); // Debounce/entry delay
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
                is_locked = false; 
                lockdown_timer.stop(); 
                failed_attempts = 0; 
                led1 = OFF; 
                led2 = OFF;
                //flash LEDs to confirm reset
                led1 = led2 = led3 = ON;
                ThisThread::sleep_for(500ms);
                led1 = led2 = led3 = OFF;
            }
            continue; 
        }

        //Compare the entered password
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            if (entered_code[i] != password[i]) {
                correct = false; 
                break;
            }
        }

        if (correct) {
            //Flash LED1 and reset failures
            led1 = ON; led2 = OFF; led3 = OFF;
            failed_attempts = 0;
            ThisThread::sleep_for(2s);
            led1 = OFF;
        } else {
            failed_attempts++; 
            
            //Visual feedback for incorrect
            led3 = ON; 
            ThisThread::sleep_for(1s);
            led3 = OFF;

            // Warning: 3 incorrect codes
            if (failed_attempts == 3) {
                //Blocking 30s warning period
                for (int j = 0; j < 30; j++) {
                    led2 = !led2; //Slowly blink LED
                    ThisThread::sleep_for(1000ms);
                }
                led2 = OFF;
            } 
            // Lockdown: 4th incorrect code
            else if (failed_attempts >= 4) {
                is_locked = true;
                lockdown_count++; //Increment event log
                lockdown_timer.reset();
                lockdown_timer.start();
            }
        }
    }
}
