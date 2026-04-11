#include "mbed.h"
#include "arm_book_lib.h"

AnalogIn potentiometer(A0);
AnalogIn temperatureSensor(A1);
DigitalIn gasSensor(PE_12);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
const float ADC_REF = 3.3f;
const float LM35_MV_PER_DEG = 0.01f;
const float TEMP_THRESHOLD_MIN = 20.0f;
const float TEMP_THRESHOLD_MAX = 100.0f;
float readTemperatureCelsius();
float readPotentiometerPercent();
bool readGasDetected();
void printReadings(float tempC, float potPercent, bool gasDetected, float threshold);
float calculateThreshold(float potPercent);

int main()
{
    char startMsg[] = "\r\n=== Sensor Monitoring Started ===\r\n";
    uartUsb.write(startMsg, sizeof(startMsg) - 1);

    while (true) {
        float temperatureC = readTemperatureCelsius();
        float potPercent   = readPotentiometerPercent();
        bool  gasDetected  = readGasDetected();
        float threshold = calculateThreshold(potPercent);
        printReadings(temperatureC, potPercent, gasDetected, threshold);
        delay(1000);
    }
}

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
    return !gasSensor.read();
}

float calculateThreshold(float potPercent)
{
    return TEMP_THRESHOLD_MIN + (potPercent / 100.0f) * (TEMP_THRESHOLD_MAX - TEMP_THRESHOLD_MIN);
}

void printReadings(float tempC, float potPercent, bool gasDetected, float threshold)
{
    char buffer[100];
    uartUsb.write("-----\r\n", 7);
    int len = snprintf(buffer, sizeof(buffer), " Temperature   : %.2f C\r\n", tempC);
    uartUsb.write(buffer, len);
    len = snprintf(buffer, sizeof(buffer), " Potentiometer : %.2f %%\r\n", potPercent);
    uartUsb.write(buffer, len);
    len = snprintf(buffer, sizeof(buffer), " Threshold     : %.2f C\r\n", threshold);
    uartUsb.write(buffer, len);
    if (gasDetected) {
        uartUsb.write(" Gas Sensor    : *** Gas DETECTED ***\r\n", 39);
    } else {
        uartUsb.write(" Gas Sensor    : Gas NOT Detected\r\n", 35);
    }
    uartUsb.write("-----\r\n\r\n", 9);
}
