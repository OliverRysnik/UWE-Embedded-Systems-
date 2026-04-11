#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn switch1(D2); //Simulates Gas Detection
DigitalIn switch5(D6); //Reset Alarms
DigitalOut alarmLed(LED1);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
bool gasAlarmState = OFF;
void inputsInit();
void outputsInit();
void alarmSystemUpdate();
void uartTask();

int main()
{
    inputsInit();
    outputsInit();
    
    while (true) {
        alarmSystemUpdate();
        uartTask();
    }
}

void inputsInit()
{
    switch1.mode(PullDown);
    switch5.mode(PullDown);
}

void outputsInit()
{
    alarmLed = OFF;
}

void alarmSystemUpdate()
{
    //If Switch 1 is pressed trigger the gas alarm
    if ( switch1 ) {
        if (gasAlarmState == OFF) {
            uartUsb.write( "WARNING: GAS DETECTED\r\n", 23 );
        }
        gasAlarmState = ON;
    }

    //If Switch 5 is pressed, reset the alarm
    if ( switch5 ) {
        if (gasAlarmState == ON) {
            uartUsb.write( "ALARMS RESET\r\n", 14 );
        }
        gasAlarmState = OFF;
    }

    alarmLed = gasAlarmState;
}

void uartTask()
{
    char receivedChar = '\0';
    
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
        
        //If 2 is pressed on keyboard, send Gas Alarm state
        if ( receivedChar == '2' ) {
            if ( gasAlarmState ) {
                uartUsb.write( "GAS ALARM ACTIVE\r\n", 18 );
            } else {
                uartUsb.write( "GAS ALARM CLEAR\r\n", 17 );
            }
        }
    }
}
