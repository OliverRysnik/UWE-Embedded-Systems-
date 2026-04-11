#include "mbed.h"
#include "arm_book_lib.h"

//=====[Objects]=====
AnalogIn potentiometer(A0);
AnalogIn temperatureSensor(A1);
DigitalIn gasSensor(PE_12);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Constants]=====
const float ADC_REF = 3.3f;
const float LM35_MV_PER_DEG = 0.01f;
const float TEMP_THRESHOLD_MIN = 20.0f;
const float TEMP_THRESHOLD_MAX = 100.0f;
float readTemperatureCelsius();
float readPotentiometerPercent();
bool readGasDetected();
float calculateThreshold(float potPercent);
void printReadings(float tempC, float potPercent, bool gasDetected, float threshold);
//Sensor readings against threshold and sends warnings
void evaluateSensorData(float tempC, bool gasDetected, float threshold);

//=====[Main]=====
int main()
{
    char startMsg[] = "\r\n=== Sensor Monitoring Started ===\r\n";
    uartUsb.write(startMsg, sizeof(startMsg) - 1);

    while (true) {
        float temperatureC = readTemperatureCelsius();
        float potPercent   = readPotentiometerPercent();
        bool  gasDetected  = readGasDetected();
        float threshold    = calculateThreshold(potPercent);

        printReadings(temperatureC, potPercent, gasDetected, threshold);

        //Sensor values and trigger warnings if needed
        evaluateSensorData(temperatureC, gasDetected, threshold);

        delay(1000);
    }
}

//=====[Functions]=====

float readTemperatureCelsius()
{
    float voltage = temperatureSensor.read() * ADC_REF;
    return voltage / LM35_MV_PER_DEG;
}

float readPotentiometerPercent()
{
    return potentiometer.read() * 100.0f;
}

bool readGasDetected()
{
    //Invert reading because MQ-2 digital output is active LOW
    return !gasSensor.read();
}

//Maps the 0-100% pot value to a threshold between minimum and maximum
float calculateThreshold(float potPercent)
{
    return TEMP_THRESHOLD_MIN + (potPercent / 100.0f) * (TEMP_THRESHOLD_MAX - TEMP_THRESHOLD_MIN);
}

//Compares temperature and gas readings against the threshold
//Sends a warning message if either condition is exceeded
void evaluateSensorData(float tempC, bool gasDetected, float threshold)
{
    char buffer[100];

    //If temperature exceeds the potentiometer-set threshold, trigger warning
    if (tempC > threshold) {
        int len = snprintf(buffer, sizeof(buffer), "Temperature Warning: %.2f C exceeds threshold of %.2f C\r\n", tempC, threshold);
        uartUsb.write(buffer, len);
    }

    //If gas is detected above the MQ-2 sensor level, trigger warning
    if (gasDetected) {
        uartUsb.write("Gas Warning: Gas detected above defined level\r\n", 51);
    }
}

void printReadings(float tempC, float potPercent, bool gasDetected, float threshold)
{
    char buffer[100];
    uartUsb.write("-----\r\n", 7);

    int len = snprintf(buffer, sizeof(buffer), " Temperature   : %.2f C\r\n", tempC);
    uartUsb.write(buffer, len);

    len = snprintf(buffer, sizeof(buffer), " Potentiometer : %.2f %%\r\n", potPercent);
    uartUsb.write(buffer, len);

    //Show the current dynamic threshold set by the potentiometer
    len = snprintf(buffer, sizeof(buffer), " Threshold     : %.2f C\r\n", threshold);
    uartUsb.write(buffer, len);

    if (gasDetected) {
        uartUsb.write(" Gas Sensor    : *** Gas DETECTED ***\r\n", 39);
    } else {
        uartUsb.write(" Gas Sensor    : Gas NOT Detected\r\n", 35);
    }

    uartUsb.write("-----\r\n\r\n", 9);
}
