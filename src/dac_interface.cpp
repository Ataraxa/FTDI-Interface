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
#include "../include/openSPI.h"
#include "../include/applicationSpecificUtils.h"

// Application specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256
#define TOGGLE_SWITCH_1 0x10
#define TOGGLE_SWITCH_2 0x20
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};

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

int main()
{
    printf("Code is up and running!");
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;

    #ifdef _MSC_VER 
        printf("MVSC detected.");
        Init_libMPSSE();
    #endif

    // SPI channel configuration
    channelConfSPI.ClockRate = 10000000;
    channelConfSPI.LatencyTimer = 2;
    channelConfSPI.configOptions = \
    SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    // channelConfSPI.Pin = setupPins(FALSE, TRUE);
    channelConfSPI.Pin = 0x00000000;
    ftHandle = openChannelSPI(channelConfSPI, channels, CHANNEL_TO_OPEN); 

    // -----------------------------------------------------------
    // CORE
    // -----------------------------------------------------------
    double targetV;
    uint16_t word;
    std::chrono::microseconds totalPulseTime;
    // auto nextPulse = std::chrono::high_resolution_clock::now();
    // auto nextEdge = std::chrono::high_resolution_clock::now();

    ConfigDBS config{
        .frequency = 130,
        .highAmplitudeWord = 0x9999,
        .nullWord = 0x8000,
        .lowAmplitudeWord = 0x6666,
        .highTime = std::chrono::microseconds(100),
        .interTime = std::chrono::microseconds(1),  
        .lowTime = std::chrono::microseconds(100),
        .totalTime = std::chrono::microseconds(3220), // includes buffer time between pulses     
    };

    // COMMAND BUFFER
    DWORD written;
    int bufferLoc = 0;
    uint8_t buffer[3];

    // Set CS low
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xA0;
    buffer[bufferLoc++] = 0xFF;

    // Set CS low
    // buffer[bufferLoc++] = 0x80;
    // buffer[bufferLoc++] = 0x08;
    // buffer[bufferLoc++] = 0x08;

    for (size_t i = 0; i < sizeof(buffer); i++) 
    {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::dec << "\n";  // Reset to decimal

    // Set CS low
    // buffer[bufferLoc] = 0x80;
    // buffer[bufferLoc++] = 0x00;
    // buffer[bufferLoc++] = 0x00;


    // buffer[bufferLoc] = 0x80;
    // buffer[bufferLoc++] = 0x00;
    // buffer[bufferLoc++] = 0x03;

    // buffer[bufferLoc++] = 0x11;
    // buffer[bufferLoc++] = 0x01; // LSB
    // buffer[bufferLoc++] = 0x00; // MSB

    // buffer[bufferLoc++] = 0x99;
    // buffer[bufferLoc++] = 0x99;

    // // Set CS high
    // buffer[bufferLoc++] = 0x80;
    // buffer[bufferLoc++] = 0x08;
    // buffer[bufferLoc++] = 0x03;

    for(int i=0;i<1;i++) // Generate 10 pulses
    {
        // nextPulse = std::chrono::high_resolution_clock::now() + config.totalTime;

        // ftStatus = FT_Write(ftHandle, buffer, 11, &written);
        // APP_CHECK_STATUS(ftStatus)
        // std::cout << written << std::endl;

        // nextEdge = std::chrono::high_resolution_clock::now() + config.highTime;
        // auto start = std::chrono::high_resolution_clock::now();
        // buffer[6] = 0x99;
        // buffer[7] = 0x99;
        uint8_t bufferMEGA[12] = {0x80, 0x00, 0xFF, 0x80, 0xFF, 0xFF, 0x80, 0x00, 0xFF, 0x80, 0xFF, 0xFF};
        ftStatus = FT_Write(ftHandle, bufferMEGA, 12, &written);
        std::cout << written << std::endl;
        APP_CHECK_STATUS(ftStatus);
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        // std::cout << "FT_Write took: " << duration.count() << " μs\n";

        // std::this_thread::sleep_until(nextEdge);

        // nextEdge = std::chrono::high_resolution_clock::now() + config.interTime;
        // buffer[6] = 0x80;
        // buffer[7] = 0x00;
        // ftStatus = FT_Write(ftHandle, buffer, 11, &written);
        // std::this_thread::sleep_until(nextEdge);

        // nextEdge = std::chrono::high_resolution_clock::now() + config.lowTime;
        // buffer[6] = 0x66;
        // buffer[7] = 0x66;
        // ftStatus = FT_Write(ftHandle, buffer, 11, &written); 
        // std::this_thread::sleep_until(nextEdge);

        // buffer[6] = 0x80;
        // buffer[7] = 0x00;
        // ftStatus = FT_Write(ftHandle, buffer, 11, &written);

        printf("hello world\n");

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

