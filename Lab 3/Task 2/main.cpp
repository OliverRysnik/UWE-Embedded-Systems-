#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn switch1(D2); //Simulates Gas Detection
DigitalIn switch3(D3); //Simulates Over-Temperature Detection
DigitalIn switch5(D6); //Reset Alarms
DigitalOut alarmLed(LED1);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
bool gasAlarmState = OFF;
bool tempAlarmState = OFF;
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
    switch3.mode(PullDown);
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

    //If Switch 3 is pressed trigger the over-temp alarm
    if ( switch3 ) {
        if (tempAlarmState == OFF) {
            uartUsb.write( "WARNING: TEMPERATURE TOO HIGH\r\n", 31 );
        }
        tempAlarmState = ON;
    }

    //If Switch 5 is pressed, reset the alarm
    if ( switch5 ) {
        if (gasAlarmState == ON || tempAlarmState == ON) { 
            uartUsb.write( "ALARMS RESET\r\n", 14 );
        }
        gasAlarmState = OFF;
        tempAlarmState = OFF;
    }

    //LED is ON if either alarm is active
    alarmLed = gasAlarmState || tempAlarmState;
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

        //If 3 is pressed on keyboard, send Temp Alarm state
        if ( receivedChar == '3' ) {
            if ( tempAlarmState ) {
                uartUsb.write( "TEMP ALARM ACTIVE\r\n", 19 );
            } else {
                uartUsb.write( "TEMP ALARM CLEAR\r\n", 18 );
            }
        }
    }
}
