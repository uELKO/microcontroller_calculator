#include <Arduino.h>

String g_serialInputBuffer = "";

void setup()
{
    Serial.begin(115200);
    g_serialInputBuffer.reserve(200);
}

void processData(String data)
{
    int32_t tempX, tempY;
    char tempCmd;

    int32_t index1 = data.indexOf(';');
    int32_t index2 = data.indexOf(';', index1 + 1);
    int32_t index3 = data.indexOf(';', index2 + 1);

    if (index1 != -1 && index2 != -1 && index3 != -1)
    {
        tempX = data.substring(0, index1).toInt();
        tempY = data.substring(index1 + 1, index2).toInt();
        tempCmd = data.charAt(index2 + 1);

        Serial.println(tempX);
        Serial.println(tempY);
        Serial.println(tempCmd);
    }
    else
    {
        Serial.println("Fehler beim Parsen der Daten.");
    }
}

void loop()
{
    while (Serial.available())
    {
        char inChar = (char)Serial.read();
        if (inChar == '\n')
        {
            processData(g_serialInputBuffer);
            g_serialInputBuffer = "";
        }
        else
        {
            g_serialInputBuffer += inChar;
        }
    }
}