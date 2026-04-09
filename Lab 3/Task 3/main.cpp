//=====[Libraries]=====

#include "mbed.h"
#include "arm_book_lib.h"

//=====[Declaration and initialization of public global objects]=====

//Buttons for the board to simulate switches
DigitalIn switch1(D2); //Simulates Gas Detection
DigitalIn switch3(D3); //Simulates Over-Temperature Detection
DigitalIn switch4(D4); //Toggle Over-Temperature Alarm
DigitalIn switch5(D6); //Reset Alarms
DigitalIn switch6(D7); //Enter/Exit Mode
DigitalOut alarmLed(LED1);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=====

bool gasAlarmState = OFF;
bool tempAlarmState = OFF;
bool lastSwitch4State = LOW; 
bool lastSwitch6State = LOW;     
bool monitoringModeActive = OFF; //To track if monitor is ON
int monitoringCounter = 0;       //For the2-second delay

//=====[Declarations (prototypes) of public functions]=====

void inputsInit();
void outputsInit();
void alarmSystemUpdate();
void uartTask();

//=====[Main function]=====

int main()
{
    inputsInit();
    outputsInit();
    
    while (true) {
        alarmSystemUpdate();
        uartTask();
        delay(10); //Delay for counter
    }
}

//=====[Implementations of public functions]=====

void inputsInit()
{
    switch1.mode(PullDown);
    switch3.mode(PullDown);
    switch4.mode(PullDown);
    switch5.mode(PullDown);
    switch6.mode(PullDown);
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

    //If Switch 4 is pressed toggle logic
    if ( switch4 == HIGH && lastSwitch4State == LOW ) {
        tempAlarmState = !tempAlarmState;
        if (tempAlarmState == ON) {
            uartUsb.write( "WARNING: TEMPERATURE TOO HIGH\r\n", 31 );
        } else {
            uartUsb.write( "TEMP ALARM CLEAR\r\n", 18 );
        }
    }
    lastSwitch4State = switch4;

    //If Switch 6 is pressed toggle logic for Monitor Mode
    if ( switch6 == HIGH && lastSwitch6State == LOW ) {
        monitoringModeActive = !monitoringModeActive;
        if (monitoringModeActive) {
            uartUsb.write( "MONITORING MODE: ON\r\n", 21 );
        } else {
            uartUsb.write( "MONITORING MODE: OFF\r\n", 22 );
        }
    }
    lastSwitch6State = switch6;

    //Continuous monitoring logic
    if ( monitoringModeActive ) {
        monitoringCounter++;
        if ( monitoringCounter >= 200 ) { //2 seconds
            if ( gasAlarmState ) uartUsb.write( "GAS: ACTIVE | ", 14 );
            else uartUsb.write( "GAS: CLEAR  | ", 14 );

            if ( tempAlarmState ) uartUsb.write( "TEMP: ACTIVE\r\n", 14 );
            else uartUsb.write( "TEMP: CLEAR \r\n", 14 );
            
            monitoringCounter = 0; //Reset timer
        }
    } else {
        monitoringCounter = 0; //Keep counter at 0 if monitor off
    }

    //If Switch 5 is pressed, reset the alarm
    if ( switch5 ) {
        if (gasAlarmState == ON || tempAlarmState == ON) {
            uartUsb.write( "ALARMS RESET\r\n", 14 );
        }
        gasAlarmState = OFF;
        tempAlarmState = OFF; 
    }

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
