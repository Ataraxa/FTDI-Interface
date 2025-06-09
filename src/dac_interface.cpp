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
    FT_HANDLE ftHandle;
    FT_STATUS ftStatus;

    #ifdef _MSC_VER 
        printf("MVSC detected.");
        Init_libMPSSE();
    #endif

    // SPI channel configuration
    channelConfSPI.ClockRate = 1000;
    channelConfSPI.LatencyTimer = 2;
    channelConfSPI.configOptions = \
    SPI_CONFIG_OPTION_MODE3 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channelConfSPI.Pin = setupPins(FALSE, TRUE);
    // ftHandle = openChannelSPI(channeConfSPI, channels, CHANNEL_TO_OPEN); 

    // -----------------------------------------------------------
    // CORE
    // -----------------------------------------------------------
    double targetV;
    uint16_t word;
    std::chrono::microseconds totalPulseTime;
    auto nextPulse = std::chrono::high_resolution_clock::now();
    auto nextEdge = std::chrono::high_resolution_clock::now();

    ConfigDBS config{
        .frequency = 130,
        .lowAmplitudeWord = 0x00,
        .highAmplitudeWord = 0x80,
        .highTime = std::chrono::microseconds(100),
        .interTime = std::chrono::microseconds(20),
        .lowTime = std::chrono::microseconds(100),
        .totalTime = std::chrono::microseconds(220),     
    };

    for(int i=0;i<10;i++) // Generate 10 pulses
    {
        nextPulse = std::chrono::high_resolution_clock::now() + config.totalTime;

        nextEdge = std::chrono::high_resolution_clock::now() + config.highTime;
        word = config.highAmplitudeWord; // High code
        writeWord(ftHandle, word);
        std::this_thread::sleep_until(nextEdge);

        nextEdge = std::chrono::high_resolution_clock::now() + config.interTime;
        word = config.nullWord;
        writeWord(ftHandle, word);
        std::this_thread::sleep_until(nextEdge);

        nextEdge = std::chrono::high_resolution_clock::now() + config.lowTime;
        word = config.lowAmplitudeWord;
        writeWord(ftHandle, word);
        std::this_thread::sleep_until(nextEdge);

        word = config.nullWord;
        writeWord(ftHandle, word);

        std::this_thread::sleep_until(nextPulse);

    }

    return 0;
}

