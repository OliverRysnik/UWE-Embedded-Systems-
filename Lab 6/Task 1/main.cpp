#include "mbed.h"
#include "arm_book_lib.h"
#include "display.h"
#include <stdint.h>

#define KEYPAD_NUMBER_OF_ROWS     4
#define KEYPAD_NUMBER_OF_COLS     4
#define DEBOUNCE_KEY_TIME_MS      40
#define TIME_INCREMENT_MS         10
#define LCD_REFRESH_MS            500     
#define TEMP_WARNING_THRESHOLD_C  31.0f   

typedef enum {
    MATRIX_KEYPAD_SCANNING,
    MATRIX_KEYPAD_DEBOUNCE,
    MATRIX_KEYPAD_KEY_HOLD_PRESSED
} matrixKeypadState_t;

AnalogIn   temperatureSensor(A1);
DigitalIn  gasSensor(PE_12);
DigitalOut keypadRowPins[KEYPAD_NUMBER_OF_ROWS] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn  keypadColPins[KEYPAD_NUMBER_OF_COLS]  = {PB_12, PB_13, PB_15, PC_6};
const float ADC_REF          = 3.3f;
const float LM35_MV_PER_DEG  = 0.01f;
int  accumulatedDebounceMatrixKeypadTime = 0;
matrixKeypadState_t matrixKeypadState;
char matrixKeypadLastKeyPressed = '\0';
int lcdRefreshAccumulator    = 0;
char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D',
};

float readTemperatureCelsius();
bool  readGasDetected();
void matrixKeypadInit();
char matrixKeypadScan();
char matrixKeypadUpdate();
void lcdDisplayUpdate(float tempC, bool gas);  
void lcdShowGasReport(bool gas);      
void lcdShowTempReport(float tempC, float threshold); 

int main()
{
    displayInit(DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER);
    matrixKeypadInit();
    while (true) {
        float temperatureC = readTemperatureCelsius();
        bool  gasDetected  = readGasDetected();
        char keyReleased = matrixKeypadUpdate();
        if (keyReleased != '\0') {
            if (keyReleased == '4') {
                lcdShowGasReport(gasDetected);
                delay(3000);
            } else if (keyReleased == '5') {
                lcdShowTempReport(temperatureC, TEMP_WARNING_THRESHOLD_C);
                delay(3000);
            }
        }

        lcdRefreshAccumulator += TIME_INCREMENT_MS;
        if (lcdRefreshAccumulator >= LCD_REFRESH_MS) {
            lcdDisplayUpdate(temperatureC, gasDetected);
            lcdRefreshAccumulator = 0;
        }
        delay(TIME_INCREMENT_MS);
    }
}

float readTemperatureCelsius()
{
    return (temperatureSensor.read() * ADC_REF) / LM35_MV_PER_DEG;
}

bool readGasDetected()
{
    return !gasSensor.read();
}

void lcdDisplayUpdate(float tempC, bool gas) {
    char line[21];
    displayCharPositionWrite(0, 0);
    displayStringWrite("== SYSTEM MONITOR ==");
    displayCharPositionWrite(0, 1);
    snprintf(line, sizeof(line), "Temp: %5.1f C      ", tempC);
    displayStringWrite(line);
    displayCharPositionWrite(0, 2);
    snprintf(line, sizeof(line), "Gas:  %-14s", gas ? "DETECTED!" : "Clear");
    displayStringWrite(line);
    displayCharPositionWrite(0, 3);
    displayStringWrite("Press [4] or [5]    ");
}

void lcdShowGasReport(bool gas) {
    displayCharPositionWrite(0, 0);
    displayStringWrite("== GAS REPORT =====");
    displayCharPositionWrite(0, 2);
    displayStringWrite(gas ? "Status: GAS DETECTED" : "Status: Clear / Safe");
}

void lcdShowTempReport(float tempC, float threshold) {
    char line[21];
    displayCharPositionWrite(0, 0);
    displayStringWrite("== TEMP REPORT =====");
    displayCharPositionWrite(0, 1);
    snprintf(line, sizeof(line), "Current:%6.1f C    ", tempC);
    displayStringWrite(line);
    displayCharPositionWrite(0, 2);
    snprintf(line, sizeof(line), "Limit:  %6.1f C    ", threshold);
    displayStringWrite(line);
    displayCharPositionWrite(0, 3);
    displayStringWrite(tempC > threshold ? "Status: OVER TEMP!  " : "Status: Normal      ");
}

void matrixKeypadInit() {
    matrixKeypadState = MATRIX_KEYPAD_SCANNING;
    for (int i = 0; i < KEYPAD_NUMBER_OF_COLS; i++) keypadColPins[i].mode(PullUp);
}

char matrixKeypadScan() {
    for (int r = 0; r < KEYPAD_NUMBER_OF_ROWS; r++) {
        for (int i = 0; i < KEYPAD_NUMBER_OF_ROWS; i++) keypadRowPins[i] = ON;
        keypadRowPins[r] = OFF;
        for (int c = 0; c < KEYPAD_NUMBER_OF_COLS; c++) {
            if (keypadColPins[c] == OFF)
                return matrixKeypadIndexToCharArray[r * KEYPAD_NUMBER_OF_COLS + c];
        }
    }
    return '\0';
}

char matrixKeypadUpdate() {
    char keyDetected = '\0';
    char keyReleased = '\0';
    switch (matrixKeypadState) {
        case MATRIX_KEYPAD_SCANNING:
            keyDetected = matrixKeypadScan();
            if (keyDetected != '\0') {
                matrixKeypadLastKeyPressed = keyDetected;
                accumulatedDebounceMatrixKeypadTime = 0;
                matrixKeypadState = MATRIX_KEYPAD_DEBOUNCE;
            }
            break;
        case MATRIX_KEYPAD_DEBOUNCE:
            if (accumulatedDebounceMatrixKeypadTime >= DEBOUNCE_KEY_TIME_MS) {
                if (matrixKeypadScan() == matrixKeypadLastKeyPressed) matrixKeypadState = MATRIX_KEYPAD_KEY_HOLD_PRESSED;
                else matrixKeypadState = MATRIX_KEYPAD_SCANNING;
            }
            accumulatedDebounceMatrixKeypadTime += TIME_INCREMENT_MS;
            break;
        case MATRIX_KEYPAD_KEY_HOLD_PRESSED:
            if (matrixKeypadScan() != matrixKeypadLastKeyPressed) {
                keyReleased = matrixKeypadLastKeyPressed;
                matrixKeypadState = MATRIX_KEYPAD_SCANNING;
            }
            break;
    }
    return keyReleased;
}
