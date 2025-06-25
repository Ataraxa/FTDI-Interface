#include <cstdio>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>
#include <bitset>
#include <array>
#include <filesystem>
#include <iostream>
#include <random>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

// Application specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256
#define TOGGLE_SWITCH_1 0x10
#define TOGGLE_SWITCH_2 0x20
#define APP_CHECK_STATUS(exp)                              \
    {                                                      \
        if (exp != FT_OK)                                  \
        {                                                  \
            printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",                                               \
                   __FILE__, __LINE__, __FUNCTION__, exp); \
            exit(1);                                       \
        }                                                  \
        else                                               \
        {                                                  \
            ;                                              \
        }                                                  \
    };

namespace fs = std::filesystem;

// -------------------------------------------------------------------------
// GLOBAL VARIABLES
// -------------------------------------------------------------------------
// Related to writing
uint32_t sizeToTransfer = 16;
uint32_t transferOptions = SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_SIZE_IN_BITS;

// Related to channel opening
ChannelConfig channelConfSPI;
DWORD channels;

void printBuffer(const uint8_t *buffer, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::dec << "\n";
}

void parseBuffer(BYTE* buffer,
    DWORD &bufferLoc, 
    uint16_t waitTime1, 
    uint16_t waitTime2, 
    uint16_t waitTime3,
    uint16_t codeHigh,
    uint16_t codeLow)
{
    bufferLoc = 0;

    // High pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;

    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = (codeHigh >> 8 ) & 0xFF;
    buffer[bufferLoc++] = codeHigh & 0xFF;
    
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;

    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime1 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime1;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }

    // Inter pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;

    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x00;

    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;

    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime2 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime2;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }

    // Low pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;

    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = (codeLow >> 8) & 0xFF;
    buffer[bufferLoc++] = codeLow & 0xFF;

    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;

    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime3 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime3;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }

    // Reset to null
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;

    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x00;

    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;
}

int main()
{
    printf("Code is up and running!");
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;
    DWORD bytesToRead;
    DWORD bytesRead;
    DWORD bufferLoc;
    DWORD written;
    BYTE buffer[2048]; 
    BYTE inputBuffer[16];

    // SPI channel configuration
    ftStatus = FT_Open(0, &ftHandle);
    APP_CHECK_STATUS(ftStatus);

    // Configure port parameters
    printf("\nConfiguring port for MPSSE use...\n");
    ftStatus |= FT_ResetDevice(ftHandle);
    //Purge USB receive buffer first by reading out all old data from FT2232H receive buffer
    ftStatus |= FT_GetQueueStatus(ftHandle, &bytesToRead);
    ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65536); //Set USB request transfer sizes to 64K
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0); //Disable event and error characters
    ftStatus |= FT_SetTimeouts(ftHandle, 0, 5000); //Sets the read and write timeouts in milliseconds
    ftStatus |= FT_SetLatencyTimer(ftHandle, 1); //Set the latency timer to 1mS (default is 16mS)
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x00, 0x00);
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00); // Reset controller
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02); //Enable MPSSE mode
    APP_CHECK_STATUS(ftStatus);
    Sleep(50); // Wait for all the USB stuff to complete and work

    // Configure SPI port
    bufferLoc = 0; // Start with a fresh index
    buffer[bufferLoc++] = 0x8A; // Use 60MHz master clock (disable divide by 5)
    buffer[bufferLoc++] = 0x97; // Turn off adaptive clocking (may be needed for ARM)
    buffer[bufferLoc++] = 0x8D; // Disable three-phase clocking
    ftStatus = FT_Write(ftHandle, buffer, bufferLoc, &written);
    APP_CHECK_STATUS(ftStatus);

    // Configure SCLK
    bufferLoc = 0; 
    buffer[bufferLoc++] = 0x86;  // Command to set clock divisor
    buffer[bufferLoc++] = 0x02; // Set 0xValueL of clock divisor
    buffer[bufferLoc++] = 0x00; // Set 0xValueH of clock divisor
    ftStatus = FT_Write(ftHandle, buffer, bufferLoc, &written); // Send off the clock divisor comm
    APP_CHECK_STATUS(ftStatus);


    // Set initial states of the MPSSE interface
    // - low byte, both pin directions and output values
    // Pin name  Signal     Dir     Config Initial Config
    // ADBUS0    TCK/SK     output  1      high    1
    // ADBUS1    TDI/DO     output  1      low     0
    // ADBUS2    TDO/DI     input   0              0
    // ADBUS3    TMS/CS     output  1      high    1
    // ADBUS4    GPIOL0     output  1      low     0
    // ADBUS5    GPIOL1     output  1      low     0
    // ADBUS6    GPIOL2     output  1      high    1
    // ADBUS7    GPIOL3     output  1      high    1
    bufferLoc = 0; // Reset output buffer pointer
    buffer[bufferLoc++] = 0x80; // Configure data bits low-byte of MPSSE port
    buffer[bufferLoc++] = 0xC8; // Initial state config above
    buffer[bufferLoc++] = 0xFB; // Direction config above
    ftStatus = FT_Write(ftHandle, buffer, bufferLoc, &written); // Send off the low GPIO config commands
    APP_CHECK_STATUS(ftStatus);

    Sleep(50);


    // -----------------------------------------------------------
    // CORE
    // -----------------------------------------------------------
   
    bufferLoc = 0;

    uint16_t wait1 = 100;
    uint16_t wait2 = 100;
    uint16_t wait3 = 100;
    parseBuffer(buffer, bufferLoc, wait1, wait2, wait3,0xA666,0x599A);

    FT_Write(ftHandle, buffer, bufferLoc, &written);

    // FT_Write(ftHandle, buffer, bufferLoc, &written);
    // // std::this_thread::sleep_for(std::chrono::microseconds(10));

 
    // ftStatus = FT_Write(ftHandle, buffer, bufferLoc, &written);
    std::cout << written << std::endl;
    APP_CHECK_STATUS(ftStatus);
    std::this_thread::sleep_for(std::chrono::microseconds(10));

    printf("hello world\n");

    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // -----------------------------------------------------------
    // Start closing everything down
    // -----------------------------------------------------------
    printf("\nAN_135 example program executed successfully.\n");
    // printf("Press <Enter> to continue\n");
    // getchar(); // wait for a carriage return
    FT_SetBitMode(ftHandle, 0x0, 0x00);
    // Reset MPSSE
    FT_Close(ftHandle); // Close the port
    return 0; // Exit with success

    return 0;
}
