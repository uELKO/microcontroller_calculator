// import librarys
#include <Arduino.h>

// global buffer for serial data bytes (chars)
String g_serialInputBuffer = "";

void setup()
{
    // start serial interface with 115200 baud rate
    Serial.begin(115200);

    // reserve RAM for serial data
    g_serialInputBuffer.reserve(200);
}

// add numbers x & y and return result
int32_t math_add(int16_t x, int16_t y)
{
    return x + y;
}

// substract number y from x and return result
int32_t math_subtract(int16_t x, int16_t y)
{
    return x - y;
}

// multiply numbers x & y and return result
int32_t math_multiply(int16_t x, int16_t y)
{
    return x * y;
}

// divide number x by y and return result
double math_divide(int16_t x, int16_t y)
{
    return x / y;
}

// calculate x to the power of y and return result
int32_t math_power(int16_t x, int16_t y)
{
    return pow(x, y);
}

// process complete input string and send result
void processData(String data)
{
    // get position of semicolons in input string
    int index1 = data.indexOf(';');
    int index2 = data.indexOf(';', index1 + 1);
    int index3 = data.indexOf(';', index2 + 1);

    // search input string for 3 semicolons; -1 if not found
    if (index1 != -1 && index2 != -1 && index3 != -1)
    {
        // buffer variables
        int32_t tempX, tempY;
        char tempCmd;

        // split string into substrings & convert data type
        tempX = data.substring(0, index1).toInt();
        tempY = data.substring(index1 + 1, index2).toInt();
        tempCmd = data.charAt(index2 + 1);

        // result variables (integer/double)
        int32_t result_i = 0;
        double result_d = 0;
        bool result_int = true;

        // call according math function for command character
        switch (tempCmd)
        {
        case 'a':
            result_i = math_add(tempX, tempY);
            result_int = true;
            break;

        case 's':
            result_i = math_subtract(tempX, tempY);
            result_int = true;
            break;

        case 'm':
            result_d = math_multiply(tempX, tempY);
            result_int = false;
            break;

        case 'd':
            result_d = math_divide(tempX, tempY);
            result_int = false;
            break;

        case 'p':
            result_d = math_power(tempX, tempY);
            result_int = true;
            break;

        default:
            Serial.println("Error: Calculation operand invalid.");
            break;
        }

        // print out result according to number type
        if (result_int == true)
        {
            Serial.println(result_i);
        }
        else
        {
            Serial.println(result_d);
        }
    }
    else
    {
        Serial.println("Error: Structure of the data package incorrect.");
    }
}

void loop()
{
    // while serial buffer has data
    while (Serial.available())
    {   
        // store byte in character
        char receivedChar = (char)Serial.read();

        // end condition (new line) ->  process data
        if (receivedChar == '\n')
        {
            processData(g_serialInputBuffer);
            g_serialInputBuffer = "";
        }

        // add character to input buffer
        else
        {
            g_serialInputBuffer += receivedChar;
        }
    }
}