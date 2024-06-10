#include <Arduino.h>

String g_serialInputBuffer = "";

void setup()
{
    Serial.begin(115200);
    g_serialInputBuffer.reserve(200);
}

int32_t math_add(int16_t x, int16_t y) {
  return x + y;
}

int32_t math_subtract(int16_t x, int16_t y) {
  return x - y;
}

int32_t math_multiply(int16_t x, int16_t y) {
  return x * y;
}

double math_divide(int16_t x, int16_t y) {
  return x / y;
}

double math_power(int16_t x, int16_t y) {
  return pow(x, y);
}

void processData(String data)
{
    int32_t tempX, tempY;
    char tempCmd;

    int index1 = data.indexOf(';');
    int index2 = data.indexOf(';', index1 + 1);
    int index3 = data.indexOf(';', index2 + 1);

    if (index1 != -1 && index2 != -1 && index3 != -1)
    {
        tempX = data.substring(0, index1).toInt();
        tempY = data.substring(index1 + 1, index2).toInt();
        tempCmd = data.charAt(index2 + 1);

        int32_t result_i = 0;
        double result_d = 0;

        switch (tempCmd)
        {
        case 'a':
            result_i = math_add(tempX, tempY);
            break;

        case 's':
            result_i = math_subtract(tempX, tempY);
            break;

        case 'm':
            result_d = math_multiply(tempX, tempY);
            break;

        case 'd':
            result_d = math_divide(tempX, tempY);
            break;

        case 'p':
            result_d = math_power(tempX, tempY);
            break;

        default:
            break;
        }

        if(tempCmd == 'a' || tempCmd == 's' || tempCmd == 'm' )
        {
            Serial.println(result_i);
        }
        else if(tempCmd == 'd' || tempCmd == 'p')
        {
            Serial.println(result_d);
        }
        else
        {
            Serial.println("Error: Calculation operand invalid.");
        }
    }
    else
    {
        Serial.println("Error: Structure of the data package incorrect.");
    }
}

void loop()
{
    while (Serial.available())
    {
        char receivedChar = (char)Serial.read();
        if (receivedChar == '\n')
        {
            processData(g_serialInputBuffer);
            g_serialInputBuffer = "";
        }
        else
        {
            g_serialInputBuffer += receivedChar;
        }
    }
}