#include "mbed.h"
#include "arm_book_lib.h"

#define NUMBER_OF_KEYS            4
#define KEYPAD_NUMBER_OF_ROWS     4
#define KEYPAD_NUMBER_OF_COLS     4
#define DEBOUNCE_KEY_TIME_MS      40
#define TIME_INCREMENT_MS         10
#define LIVE_FEED_DELAY_MS        2000
#define MAX_EVENTS                5

typedef enum {
    MATRIX_KEYPAD_SCANNING,
    MATRIX_KEYPAD_DEBOUNCE,
    MATRIX_KEYPAD_KEY_HOLD_PRESSED
} matrixKeypadState_t;

struct EventLog {
    float temperature;
    bool gasDetected;
};

EventLog eventLogs[MAX_EVENTS];
int nextLogIndex = 0;
int totalEventsLogged = 0;

AnalogIn potentiometer(A0);
AnalogIn temperatureSensor(A1);
DigitalIn gasSensor(PE_12);
DigitalOut buzzer(PE_10);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
DigitalOut keypadRowPins[KEYPAD_NUMBER_OF_ROWS] = {PB_3, PB_5, PC_7, PA_15};
DigitalIn keypadColPins[KEYPAD_NUMBER_OF_COLS]  = {PB_12, PB_13, PB_15, PC_6};
const float ADC_REF = 3.3f;
const float LM35_MV_PER_DEG = 0.01f;
bool alarmActive = false;
bool liveFeedEnabled = false;
int liveFeedAccumulator = 0;
char codeSequence[NUMBER_OF_KEYS] = { '1', '8', '0', '5' };
char keyPressed[NUMBER_OF_KEYS]   = { '0', '0', '0', '0' };
int matrixKeypadCodeIndex = 0;
int accumulatedDebounceMatrixKeypadTime = 0;
matrixKeypadState_t matrixKeypadState;
char matrixKeypadLastKeyPressed = '\0';
char matrixKeypadIndexToCharArray[] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D',
};

float readTemperatureCelsius();
float readPotentiometerPercent();
bool readGasDetected();
float calculateTempThreshold(float potPercent);
float calculateAirQualityThreshold(float potPercent);
void matrixKeypadInit();
char matrixKeypadScan();
char matrixKeypadUpdate();
bool checkCode();
void uartTask();
void displayMenu();
void logEvent(float temp, bool gas);
void displayLog();

int main() {
    buzzer = LOW;
    matrixKeypadInit();
    displayMenu();

    while (true) {
        float temperatureC = readTemperatureCelsius();
        float potPercent   = readPotentiometerPercent();
        bool  gasDetected  = readGasDetected();
        float tempThreshold = calculateTempThreshold(potPercent);
        float airThreshold  = calculateAirQualityThreshold(potPercent);

        if (temperatureC > tempThreshold || gasDetected) {
            if (!alarmActive) {
                logEvent(temperatureC, gasDetected);
                uartUsb.write("\r\n[ALARM] Threshold Exceeded! Enter 4-Digit Code to Deactivate\r\n", 63);
            }
            alarmActive = true;
        }

        buzzer = alarmActive ? HIGH : LOW;
        char keyReleased = matrixKeypadUpdate();
        if (keyReleased != '\0') {
            if (keyReleased == '#') {
                displayLog();
                if (checkCode()) {
                    alarmActive = false;
                    uartUsb.write("\r\n[SYSTEM] Alarm Deactivated Successfully\r\n", 43);
                } else {
                    uartUsb.write("\r\n[ERROR] Incorrect Code Please Try again\r\n", 37);
                }
                matrixKeypadCodeIndex = 0;
            } else {
                if (matrixKeypadCodeIndex < NUMBER_OF_KEYS) {
                    keyPressed[matrixKeypadCodeIndex] = keyReleased;
                    matrixKeypadCodeIndex++;
                    uartUsb.write("*", 1);
                }
            }
        }
        uartTask();
        if (liveFeedEnabled) {
            liveFeedAccumulator += TIME_INCREMENT_MS;
            if (liveFeedAccumulator >= LIVE_FEED_DELAY_MS) {
                char buffer[150];
                int len = snprintf(buffer, sizeof(buffer), 
                    "\r\n--- LIVE FEED ---\r\n"
                    "TEMP: %.2f C (Thresh: %.2f C)\r\n"
                    "AIR THRESHOLD: %.0f ppm\r\n"
                    "GAS: %s | ALARM: %s\r\n", 
                    temperatureC, tempThreshold, airThreshold, 
                    gasDetected ? "DETECTED" : "Clear", alarmActive ? "ACTIVE" : "OFF");
                uartUsb.write(buffer, len);
                liveFeedAccumulator = 0; 
            }
        }
        delay(TIME_INCREMENT_MS);
    }
}

void logEvent(float temp, bool gas) {
    eventLogs[nextLogIndex].temperature = temp;
    eventLogs[nextLogIndex].gasDetected = gas;
    nextLogIndex = (nextLogIndex + 1) % MAX_EVENTS;
    if (totalEventsLogged < MAX_EVENTS) {
        totalEventsLogged++;
    }
}

void displayLog() {
    char buffer[100];
    uartUsb.write("\r\n========== RECENT EVENT LOG ==========\r\n", 41);
    if (totalEventsLogged == 0) {
        uartUsb.write("No events recorded.\r\n", 21);
    } else {
        for (int i = 0; i < totalEventsLogged; i++) {
            int index = (totalEventsLogged < MAX_EVENTS) ? i : (nextLogIndex + i) % MAX_EVENTS;
            int len = snprintf(buffer, sizeof(buffer), "Event %d: Temp: %.2f C, Gas: %s\r\n", 
                               i + 1, eventLogs[index].temperature, 
                               eventLogs[index].gasDetected ? "YES" : "NO");
            uartUsb.write(buffer, len);
        }
    }
    uartUsb.write("======================================\r\n", 40);
}

float calculateTempThreshold(float potPercent) {
    return 25.0f + (potPercent / 100.0f) * (37.0f - 25.0f);
}

float calculateAirQualityThreshold(float potPercent) {
    return (potPercent / 100.0f) * 800.0f;
}

void displayMenu() {
    char msg[] = 
        "\r\n==========================================\r\n"
        "      EMBEDDED CONTROL SYSTEM HUB       \r\n"
        "==========================================\r\n"
        " DEACTIVATION CODE : 1 8 0 5 #           \r\n"
        " KEYBINDS:                               \r\n"
        "  [L] : Toggle Live Feed ON/OFF          \r\n"
        "  [M] : Show this Menu again             \r\n"
        "  [#] : (Keypad) View Event Log          \r\n"
        "==========================================\r\n";
    uartUsb.write(msg, sizeof(msg) - 1);
}

void uartTask() {
    char receivedChar = '\0';
    if (uartUsb.readable()) {
        uartUsb.read(&receivedChar, 1);
        if (receivedChar == 'l' || receivedChar == 'L') {
            liveFeedEnabled = !liveFeedEnabled;
            if (liveFeedEnabled) {
                uartUsb.write("\r\nLive Feed: ENABLED\r\n", 21);
            } else {
                uartUsb.write("\r\nLive Feed: DISABLED\r\n", 22);
            }
        } else if (receivedChar == 'm' || receivedChar == 'M') {
            displayMenu();
        }
    }
}

float readTemperatureCelsius() {
    return (temperatureSensor.read() * ADC_REF) / LM35_MV_PER_DEG;
}

float readPotentiometerPercent() {
    return potentiometer.read() * 100.0f;
}

bool readGasDetected() {
    return !gasSensor.read();
}

bool checkCode() {
    for (int i = 0; i < NUMBER_OF_KEYS; i++) {
        if (codeSequence[i] != keyPressed[i]) return false;
    }
    return true;
}

void matrixKeypadInit() {
    matrixKeypadState = MATRIX_KEYPAD_SCANNING;
    for (int i = 0; i < KEYPAD_NUMBER_OF_COLS; i++) {
        keypadColPins[i].mode(PullUp);
    }
}

char matrixKeypadScan() {
    for (int r = 0; r < KEYPAD_NUMBER_OF_ROWS; r++) {
        for (int i = 0; i < KEYPAD_NUMBER_OF_ROWS; i++) keypadRowPins[i] = ON;
        keypadRowPins[r] = OFF;
        for (int c = 0; c < KEYPAD_NUMBER_OF_COLS; c++) {
            if (keypadColPins[c] == OFF) return matrixKeypadIndexToCharArray[r * KEYPAD_NUMBER_OF_COLS + c];
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
                if (matrixKeypadScan() == matrixKeypadLastKeyPressed) {
                    matrixKeypadState = MATRIX_KEYPAD_KEY_HOLD_PRESSED;
                } else {
                    matrixKeypadState = MATRIX_KEYPAD_SCANNING;
                }
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
