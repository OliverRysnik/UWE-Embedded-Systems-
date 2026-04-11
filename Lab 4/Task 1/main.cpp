#include "mbed.h"
#include "arm_book_lib.h"

AnalogIn potentiometer(A0);
AnalogIn temperatureSensor(A1);
DigitalIn gasSensor(PE_12);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
const float ADC_REF = 3.3f;
const float LM35_MV_PER_DEG = 0.01f;
float readTemperatureCelsius();
float readPotentiometerPercent();
bool readGasDetected();
void printReadings(float tempC, float potPercent, bool gasDetected);

int main()
{
    char startMsg[] = "\r\n=== Sensor Monitoring Started ===\r\n";
    uartUsb.write(startMsg, sizeof(startMsg) - 1);

    while (true) {
        float temperatureC = readTemperatureCelsius();
        float potPercent   = readPotentiometerPercent();
        bool  gasDetected  = readGasDetected();

        printReadings(temperatureC, potPercent, gasDetected);
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

void printReadings(float tempC, float potPercent, bool gasDetected)
{
    char buffer[100];
    uartUsb.write("-----\r\n", 7);
    int len = snprintf(buffer, sizeof(buffer), " Temperature   : %.2f C\r\n", tempC);
    uartUsb.write(buffer, len);
    len = snprintf(buffer, sizeof(buffer), " Potentiometer : %.2f %%\r\n", potPercent);
    uartUsb.write(buffer, len);
    if (gasDetected) {
        uartUsb.write(" Gas Sensor    : *** Gas DETECTED ***\r\n", 39);
    } else {
        uartUsb.write(" Gas Sensor    : Gas NOT Detected\r\n", 35);
    }

    uartUsb.write("-----\r\n\r\n", 9);
}
