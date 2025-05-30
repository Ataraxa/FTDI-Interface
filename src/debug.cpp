#include <cstdio>
#include <chrono>
#include <thread>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

void InitialiseCommunication(FT_STATUS& status, FT_HANDLE& ftHandle)
{
    static FT_DEVICE_LIST_INFO_NODE *devInfo;
    static DWORD dwNumDevs;

    // Check all connected devices
    printf("Checking for FTDI devices...\n");
    status = FT_CreateDeviceInfoList(&dwNumDevs);

    // Gather information about detected devices
    devInfo =(FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*dwNumDevs);
    status = FT_GetDeviceInfoList(devInfo,&dwNumDevs);

    // Open communication with channel A
    int deviceIndex = 0;
    char* descr = devInfo[deviceIndex].Description;
    printf("Opening Channel %s of FTDI device... \n", descr);
    status = FT_Open(deviceIndex, &ftHandle);
}

void InitialiseSPI(FT_STATUS& ftStatus, FT_HANDLE& ftHandle)
{
    DWORD numChanSPI;

    SPI_GetNumChannels(&numChanSPI);
    printf("SPI channels: %d", numChanSPI);
}

int main(int argc, CHAR* argv[])
{
    // Namespace 
    using namespace std::this_thread;
    using namespace std::chrono_literals;
    using std::chrono::system_clock;

    // VARIABLE DECLARATION ---------------------------------
    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;

    BYTE data;

    // To make FT_READ and FT_WRITE functions work
    // DWORD dwNumBytesToRead = 0;
    // DWORD dwNumBytesRead = 0;
    // DWORD dwNumBytesToSend = 0;
    // DWORD dwNumBytesSent = 0;

    // BYTE inputBuffer[8];
    // BYTE outputBuffer[8];


    // MAIN ------------------------------------------------
    printf("Starting the program...");

    // Device Initialisation -----------
    InitialiseCommunication(ftStatus, ftHandle);
    if (ftStatus != FT_OK)
    {
        printf("Open failed");
        return 1;
    }

    // Initialize MPSSE
    Init_libMPSSE();
    // FT_ResetDevice(ftHandle);
    // FT_SetBitMode(ftHandle, 0x0, 0x00); // reset
    // FT_SetBitMode(ftHandle, 0x0, 0x02); // MPSSE mode
    // --------------------------------------------------------

    InitialiseSPI(ftStatus, ftHandle);

    // Read from FTDI device
    for (int i = 0; i < 10; i++)
    {
        ftStatus = FT_WriteGPIO(ftHandle, 0x08, 0x00);
        if (ftStatus != FT_OK)
        {
            printf("Failed...");
            return 1;
        }
        sleep_for(200ms);

        ftStatus = FT_WriteGPIO(ftHandle, 0x08, 0x08);
        if (ftStatus != FT_OK)
        {
            printf("Failed...");
            return 1;
        }
        sleep_for(200ms); 
    }

    
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