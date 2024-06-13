// import librarys
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void listSerialPorts(UINT16 length)
{
    char portName[10];
    printf("Available serial ports:\n");
    for (UINT16 i = 0; i <= length; ++i)
    {
        snprintf(portName, sizeof(portName), "COM%d", i);
        HANDLE hCom = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCom != INVALID_HANDLE_VALUE)
        {
            printf("  %s\n", portName);
            CloseHandle(hCom);
        }
    }
}

HANDLE openSerialPort(const char *portName)
{
    HANDLE hCom = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
    {
        printf("Error: serial port %s not available or busy.\n", portName);
        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams = {0}; // Device Control Block
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hCom, &dcbSerialParams))
    {
        printf("Error when retrieving the DCB structure.\n");
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(hCom, &dcbSerialParams))
    {
        printf("Error when setting the DCB structure.\n");
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 5000; // 5 seconds timeout
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hCom, &timeouts))
    {
        printf("Error when setting the timeouts.\n");
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return hCom;
}

// Funktion zum Senden von Daten über die serielle Schnittstelle
void sendSerialData(HANDLE hCom, const char *data)
{
    DWORD bytes_written;
    if (!WriteFile(hCom, data, strlen(data), &bytes_written, NULL))
    {
        printf("Error when receiving the data.\n");
    }
    else
    {
        printf("Data packet sent: %s", data);
    }
}

// Funktion zum Empfangen von Daten über die serielle Schnittstelle
char* receiveSerialData(HANDLE hCom)
{
    static char buf[100];

    DWORD bytes_read;
    BOOL result = ReadFile(hCom, buf, sizeof(buf) - 1, &bytes_read, NULL);
    if (result && bytes_read > 0)
    {
        buf[bytes_read] = '\0'; //String Null-Terminierung
        //printf("Result: %s", buf);
    }
    else if (!result)
    {
        printf("Error when receiving the data.\n");
    }
    else
    {
        printf("No data received (timeout).\n");
    }

    return buf;
}

// Funktion zum Abfragen einer Ja/Nein-Eingabe
char* getInputChar()
{
    getchar();
    static char input; //static: auch nach Funktionsaufruf noch verfügbar
    while (1)
    {
        printf(">Would you like to exit the program? (y/n): ");
        scanf("%c", &input);

        if (input == 'y' || input == 'Y')
        {
            input = 'y';
        }
        else if (input == 'n' || input == 'N')
        {
            input = 'n';
        }
        else
        {
            printf("Invalid input. Please enter 'y' or 'n'.\n");
        }
        return input;
    }
}

int main()
{
    printf("#####################################\n");
    printf("##### Windows Remote Calculator #####\n");
    printf("#####################################\n");

    printf("\n--- Initiate Serial Connection ---\n");

    listSerialPorts(256);

    printf(">Select the serial port to be opened (enter the full name, e.g. COM3): ");

    char portName[7];
    scanf("%6s", portName);

    printf("Opening serial port: %s\n", portName);
    HANDLE hCom = openSerialPort(portName);
    if (hCom != INVALID_HANDLE_VALUE)
    {
        printf("Serial port %s opened.\n", portName);
    }
    while (1)
    {
        printf("\n--- Remote Calculator ---\n");

        int x, y;
        char c;

        printf(">Enter first number (int): ");
        if (scanf("%d", &x) != 1)
        {
            fprintf(stderr, "Invalid input.\n");
            return 0;
        }

        printf(">Enter second number (int): ");
        if (scanf("%d", &y) != 1)
        {
            fprintf(stderr, "Invalid input.\n");
            return 0;
        }

        printf(">Enter operand: add (a)/ substract (s)/ multiply (m)/ divide (d)/ power (p): ");
        scanf(" %c", &c);

        char data[100];
        snprintf(data, sizeof(data), "%d;%d;%c;\n", x, y, c);
        sendSerialData(hCom, data);

        printf("------------\n");

        char input = *receiveSerialData(hCom);
        if (input != NULL) {
            printf("Result: %s", input);
        }

        printf("--------------------------------\n");

        // Warten auf Benutzeraktion
        while (1)
        {
            // Fragen, ob das Programm beendet werden soll
            char response = *getInputChar();
            if (response == 'y')
            {
                // Seriellen Port schließen
                CloseHandle(hCom);
                printf("Serial port closed.\n");
                printf("\n--- Program End ---\n");
                Sleep(1000);
                return 0;
            }
            else if (response == 'n')
            {
                break;
            }
        }
    }
}