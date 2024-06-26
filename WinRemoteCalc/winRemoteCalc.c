// Import librarys
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Constants
#define DEBUG_MODE 0

/**
 * @brief Lists all available serial (COM) ports on the computer.
 * @param UINT8 len: scan from port 0 to port <len>
 * @return void
 */
void listSerialPorts(UINT8 len)
{
    char portName[10];
    printf("Available serial ports:\n");
    for (UINT8 i = 0; i <= len; ++i)
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

/**
 * @brief Opens serial port.
 * This function opens a serial port with following input syntax: COM<0-255>
 * and sets port properties:
 * Baudrate: 115200 |
 * Byte size: 8 |
 * Stop bit: 1 |
 * Parity: no
 * @param const char *portName: complete name (e.g. COM3) of the serial interface
 * @return HANDLE (Windows API)
 */
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

/**
 * @brief Sends data via the given serial interface.
 * Prints message when write error to device occurs.
 * @param
 * hCom: serial interface |
 * const char *data: pointer to string
 * @return void
 */
void sendSerialData(HANDLE hCom, const char *data)
{
    DWORD bytes_written;
    if (!WriteFile(hCom, data, strlen(data), &bytes_written, NULL))
    {
        printf("Error when receiving the data.\n");
    }
    else
    {
        if (DEBUG_MODE)
        {
            printf("Data sent: %s", data);
            printf("(%d Bytes)\n", strlen(data));
        }
    }
}

/**
 * @brief Receives data via the given serial interface.
 * Prints message when read error from device occurs.
 * @param hCom: serial interface
 * @return char* buf: pointer to string
 */
char *receiveSerialData(HANDLE hCom)
{
    static char buf[100];

    DWORD bytes_read;
    BOOL result = ReadFile(hCom, buf, sizeof(buf) - 1, &bytes_read, NULL);
    if (result && bytes_read > 0)
    {
        buf[bytes_read] = '\0'; // String Null-Terminierung
        if (DEBUG_MODE)
        {
            printf("Data received: %s", buf);
            printf("(%d Bytes)\n", strlen(buf));
        }
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

/**
 * @brief Determines if the user wants to exit the program.
 * This function prompts the user to enter 'y' or 'n' to decide whether to exit the program.
 * @return char Returns 'y' if the user wants to exit, 'n' otherwise.
 */
char getUserInput()
{
    static char input; // Static: still available after function call
    while (1)
    {
        printf(">Would you like to exit the program? (y/n): ");
        scanf("%c", &input);

        if (DEBUG_MODE)
        {
            printf("User input: %d", input);
        }

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

/**
 * @brief Receives data via the given serial interface.
 * Prints message when read error from device occurs.
 * @param hCom: serial interface
 * @return char* buf: pointer to string
 */
int splitCalculationString(const char *input_buf, int *number_1, int *number_2, char *operator)
{
    const char *operators = "+-*/^";
    const char *operators_arduino = "asmdp"; // Map to math operator defined in Arduino firmware
    const char *ptr = input_buf;

    // Run through the string and find the operator
    while (*ptr)
    {
        const char *op_ptr = strchr(operators, *ptr); // Store
        if (op_ptr)
        {
            int index = op_ptr - operators;      // Index of the operator in "operators"
            *operator= operators_arduino[index]; // Corresponding operator from "operators_arduino"
            break;
        }
        ptr++;
    }

    if (*operator== '\0')
    {
        // No valid operator found
        return 0;
    }

    // Extract the numbers before and after the operator
    char left_part[100] = {0};
    char right_part[100] = {0};
    strncpy(left_part, input_buf, (size_t)(ptr - input_buf));
    strcpy(right_part, ptr + 1);

    // Remove leading and trailing spaces
    *number_1 = atoi(left_part);
    *number_2 = atoi(right_part);

    return 1;
}

/**
 * @brief Clears serial buffer.
 * Receives data from serial buffer until new line character.
 * @return void
 */
void clearSerialBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) // "EOF" is returned by getchar() -> avoids an infinite loop
    {
        continue;
    }
}

/// @brief
/// @return
int main()
{
    printf("#####################################\n");
    printf("##### Windows Remote Calculator #####\n");
    printf("#####################################\n");

    printf("\n--- Initiate Serial Connection ---\n");

    listSerialPorts(254);

    printf(">Select the serial port to be opened (enter the full name, e.g. COM3): ");

    char portName[7];
    scanf("%6s", portName); // get serial input

    printf("Opening serial port: %s\n", portName);
    HANDLE hCom = openSerialPort(portName);
    if (hCom != INVALID_HANDLE_VALUE)
    {
        printf("Serial port %s opened.\n", portName);
    }

    // stay in loop (recurring calculation)
    while (hCom != INVALID_HANDLE_VALUE)
    {
        printf("\n--- Remote Calculator ---\n");

        char input_buf[100];

        printf(">Enter calculation: ");
        scanf("%s", input_buf);

        int number_1, number_2;
        char operator= '\0';

        if (splitCalculationString(input_buf, &number_1, &number_2, &operator))
        {
            if (DEBUG_MODE)
            {
                printf("Number 1: %d\n", number_1);
                printf("Number 2: %d\n", number_2);
                printf("Operator: %c\n", operator);
            }

            /**
             * send string over serial interface. Syntax:
             * <number_1>;<number_2>;<calculation>;
             */
            char data[100];
            snprintf(data, sizeof(data), "%d;%d;%c;\n", number_1, number_2, operator);
            sendSerialData(hCom, data);

            printf("------------\n");

            char *returned_str = receiveSerialData(hCom);
            if (returned_str != NULL)
            {
                printf("Result: %s", returned_str);
            }
            clearSerialBuffer();
        }
        else
        {
            printf("No valid operator found in the string.\n");
            clearSerialBuffer();
        }

        printf("--------------------------------\n");

        /**
         * wait for user input
         * y: close serial port and close terminal
         * n: start new calculation
         */
        char response = getUserInput();
        if (response == 'y')
        {
            CloseHandle(hCom);
            printf("Serial port closed.\n");
            printf("\n--- Program End ---\n");
            Sleep(1000);
            return 0;
        }
    }
}