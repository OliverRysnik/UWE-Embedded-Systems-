#include "mbed.h"
#include "arm_book_lib.h"

//=====[Objects]=====
AnalogIn potentiometer(A0);
AnalogIn temperatureSensor(A1);
DigitalIn gasSensor(PE_12);
UnbufferedSerial uartUsb(USBTX, USBRX, 115200); // Set baud rate to 115200 for serial communication
const float ADC_REF = 3.3f;
const float LM35_MV_PER_DEG = 0.01f;

float readTemperatureCelsius();
float readPotentiometerPercent();
bool readGasDetected();
void printReadings(float tempC, float potPercent, bool gasDetected);

//=====[Main]=====
int main()
{
    char startMsg[] = "\r\n=== Sensor Monitoring Started ===\r\n";
    uartUsb.write(startMsg, sizeof(startMsg) - 1);

    while (true) {
        float temperatureC = readTemperatureCelsius();
        float potPercent   = readPotentiometerPercent();
        bool  gasDetected  = readGasDetected();

        printReadings(temperatureC, potPercent, gasDetected);
        delay(1000); //Wait 1 second
    }
}

//=====[Functions]=====

float readTemperatureCelsius()
{
    //Convert 0.0-1.0 analog value to voltage and then to Celsius
    float voltage = temperatureSensor.read() * ADC_REF;
    return voltage / LM35_MV_PER_DEG;
}

float readPotentiometerPercent()
{
    //Scale the raw analog input to a 0-100 percentage range
    return potentiometer.read() * 100.0f;
}

bool readGasDetected()
{
    //Invert reading because MQ-2 digital output is active LOW
    return !gasSensor.read();
}

void printReadings(float tempC, float potPercent, bool gasDetected)
{
    char buffer[100];
    uartUsb.write("-----\r\n", 7);

    //Send sensor data strings to the serial terminal
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
