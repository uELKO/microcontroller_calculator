// import librarys
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void list_serial_ports(UINT16 length)
{
    char port_name[10];
    printf("Available serial ports:\n");
    for (int i = 1; i <= length; ++i)
    {
        snprintf(port_name, sizeof(port_name), "COM%d", i);
        HANDLE hCom = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCom != INVALID_HANDLE_VALUE)
        {
            printf("%s\n", port_name);
            CloseHandle(hCom);
        }
    }
}

HANDLE open_serial_port(const char *portname)
{
    HANDLE hCom = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
    {
        printf("Error: serial port %s not available or busy.\n", portname);
        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams = {0};
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

int main()
{
    while (1)
    {
        printf("### Windows Remote Calculator ###\n");

        list_serial_ports(256);

        printf("Select the serial port to be opened (enter the full name, e.g. COM3): ");

        char portname[7];
        scanf("%6s", portname);

        printf("Opening serial port: %s\n", portname);
        HANDLE hCom = open_serial_port(portname);
        if (hCom != INVALID_HANDLE_VALUE)
        {
            printf("Serial port %s opened.\n", portname);
        }

        Sleep(5000);

        // Seriellen Port schließen
        CloseHandle(hCom);
        printf("Serial port closed.\n");

        printf("--- Program End ---\n");
    }
}