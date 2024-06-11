// import librarys
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void list_serial_ports() {
    char port_name[10];
    printf("Available serial ports:\n");
    for (int i = 1; i <= 256; ++i) {
        snprintf(port_name, sizeof(port_name), "COM%d", i);
        HANDLE hCom = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hCom != INVALID_HANDLE_VALUE) {
            printf("%s\n", port_name);
            CloseHandle(hCom);
        }
    }
}

int main() {
    list_serial_ports();
    char portname[7];
    printf("Select the serial port to be opened (enter the full name, e.g. COM3): ");
    scanf("%6s", portname);
 
    //return 0;
}