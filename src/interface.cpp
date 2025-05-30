#include<cstdio>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

int main(int argc, CHAR* argv[]) {
    // -----------------------------------------------------------
    // Variables
    // -----------------------------------------------------------

    FT_HANDLE ftHandle; // Handle of the FTDI device
    FT_STATUS ftStatus; // Result of each D2XX call

    DWORD dwNumDevs; // The number of devices
    unsigned int uiDevIndex = 0xF; // The device in the list that we'll use
    
    BYTE byOutputBuffer[8]; // Buffer to hold MPSSE commands and data to be sent to the FT2232H
    BYTE byInputBuffer[8]; // Buffer to hold data read from the FT2232H

    DWORD dwCount = 0; // General loop index
    DWORD dwNumBytesToSend = 0; // Index to the output buffer
    DWORD dwNumBytesSent = 0; // Count of actual bytes sent - used with FT_Write
    DWORD dwNumBytesToRead = 0; // Number of bytes available to read
    // in the driver's input buffer
    DWORD dwNumBytesRead = 0; // Count of actual bytes read - used with FT_Read
    DWORD dwClockDivisor = 0x05DB; // Value of clock divisor, SCL Frequency =
    // 60/((1+0x05DB)*2) (MHz) = 1Mhz


    // -----------------------------------------------------------
    // Does an FTDI device exist?
    // -----------------------------------------------------------
    printf("Checking for FTDI devices...\n");
    ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
    // Get the number of FTDI devices
    if (ftStatus != FT_OK) // Did the command execute OK?
    {
        printf("Error in getting the number of devices\n");
        return 1; // Exit with error
    }

    if (dwNumDevs < 1) // Exit if we don't see any
    {
        printf("There are no FTDI devices installed\n");
        return 1; // Exit with error
    }
    printf("%d FTDI devices found \
    - the count includes individual ports on a single chip\n", dwNumDevs);
    
    // -----------------------------------------------------------
    // Open the port - For this application note, we'll assume the first device is a
    // FT2232H or FT4232H. Further checks can be made against the device
    // descriptions, locations, serial numbers, etc. before opening the port.
    // -----------------------------------------------------------
    printf("\nAssume first device has the MPSSE and open it...\n");
    ftStatus = FT_Open(0, &ftHandle);
    if (ftStatus != FT_OK)
    {
        printf("Open Failed with error %d\n", ftStatus);
        return 1; // Exit with error
    }

    // Read from 
    dwNumBytesToSend = 0;
    ftStatus |= FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
    if ((ftStatus != FT_OK) & (dwNumBytesToRead != 1))
    {
        printf("Error - GPIO cannot be read\n");
        FT_SetBitMode(ftHandle, 0x0, 0x00);
        // Reset the port to disable MPSSE
        FT_Close(ftHandle); // Close the USB port
        return 1; // Exit with error
    }
    printf("The GPIO low-byte = 0x%X\n", byInputBuffer[0]);
    // The inpute buffer only contains one
    // valid byte at location 0
    printf("Press <Ent> to continue\n");
    getchar(); // wait for a carriage return

    byOutputBuffer[dwNumBytesToSend++] = 0x80;
    // Set data bits low-byte of MPSSE port
    byOutputBuffer[dwNumBytesToSend++] = byInputBuffer[0] & 0xF7;
    // Only change TMS/CS bit to zero
    byOutputBuffer[dwNumBytesToSend++] = 0xFB;
    // Direction config is still needed for each GPIO write
    ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
    // Send off the low GPIO config commands
    dwNumBytesToSend = 0; // Reset output buffer pointer
    Sleep(2);
    // FT_Write(ftHandle, )

    // -----------------------------------------------------------
    // Start closing everything down
    // -----------------------------------------------------------
    printf("\nAN_135 example program executed successfully.\n");
    printf("Press <Enter> to continue\n");
    getchar(); // wait for a carriage return
    FT_SetBitMode(ftHandle, 0x0, 0x00);
    // Reset MPSSE
    FT_Close(ftHandle); // Close the port
    return 0; // Exit with success
}