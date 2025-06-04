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
#include <math.h> 

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

// Application specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256
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

// Application specific variables
double Vref = 5.0;
double bitResDAC = 16;

int randomInt() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(-6, 6);
    return dist(gen);
}

FT_STATUS writeWord(FT_HANDLE ftHandle, uint16_t word)
{
    static FT_STATUS fcStatus;
    DWORD sizeTransferred = 0;

    UCHAR bytes[2] = {
        static_cast<UCHAR>((word >> 8) & 0xFF),  // High byte
        static_cast<UCHAR>(word & 0xFF)          // Low byte
    };

    fcStatus = SPI_Write(ftHandle,bytes,sizeToTransfer,&sizeTransferred,transferOptions);
    printf("Successfully transferred %u bits", sizeTransferred);
    return fcStatus;
}

uint16_t convertVoltToWord(int targetVoltage)
{
    uint16_t word;

    word = (targetVoltage+Vref)* pow(2.0, bitResDAC)/(2*Vref);

    return word;
}

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
    SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channelConfSPI.Pin = 0x00000000;

    // SPI Channel detection and opening
    // Get number of SPI channels
    ftStatus = SPI_GetNumChannels(&channels);
    APP_CHECK_STATUS(ftStatus);
    printf("There are %u SPI channels available.\n", channels);

    // Info about opened channel
    FT_DEVICE_LIST_INFO_NODE infoChanSPI;
    ftStatus = SPI_GetChannelInfo(CHANNEL_TO_OPEN, &infoChanSPI);
    printf("The opened channel has the followng properties:\n");
    printf(" Flags=0x%x\n",infoChanSPI.Flags);
    printf(" Type=0x%x\n",infoChanSPI.Type);
    printf(" ID=0x%x\n",infoChanSPI.ID);
    printf(" LocId=0x%x\n",infoChanSPI.LocId);
    printf(" SerialNumber=%s\n",infoChanSPI.SerialNumber);
    printf(" Description=%s\n",infoChanSPI.Description);
    printf(" ftHandle=0x%x\n",infoChanSPI.ftHandle);
    ftStatus = SPI_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
    APP_CHECK_STATUS(ftStatus);
    printf("\nhandle=0x%x status=0x%x\n",ftHandle,ftStatus);
    ftStatus = SPI_InitChannel(ftHandle,&channelConfSPI);
    APP_CHECK_STATUS(ftStatus);

    // -----------------------------------------------------------
    // CORE
    // -----------------------------------------------------------
    double targetV;
    uint16_t word;
    for(int i=0; i < 5; i++)
    {
        targetV = randomInt()/2.0;
        word = convertVoltToWord(targetV);
        printf("Target voltage: %f | Corresponding 16-bit code: ");
        std::cout << std::bitset<16>(word) << std::endl;

        writeWord(ftHandle, word);
    }

    return 0;
}

