#include <cstdio>
#include <chrono>
#include <thread>
#include <fstream>
#include <iostream>
#include <bitset>
#include <array>
#include <filesystem>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"
// #include "../include/optiRead.h"

// Application-specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256

namespace fs = std::filesystem;

// Useful macros 
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};

// Global variables
ChannelConfig channelConfSPI;
DWORD channels;
uint8_t buffer[SPI_DEVICE_BUFFER_SIZE];

constexpr size_t BUFFER_SIZE = 4000;
std::array<uint16_t, BUFFER_SIZE> sample_buffer{};    

FT_STATUS readSample(FT_HANDLE ftHandle, uint16_t *data)
{
    // Instantiate auxiliary variables
    static uint32_t sizeToTransfer = 0;
    static DWORD sizeTransferred = 0; 
    static FT_STATUS fcStatus;

    uint8_t buffer[3] = {};
    // std::cout << std::to_string(buffer[0]) << std::endl;
    // std::cout << std::to_string(buffer[1]) << std::endl;

    // Read 16 bits from ADC
    sizeToTransfer = 18;
    sizeTransferred = 0;
    fcStatus = SPI_Read(ftHandle, (UCHAR *)&buffer, sizeToTransfer, &sizeTransferred,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(fcStatus);

    // std::cout << std::bitset<8>(buffer[0]) << "|" << std::bitset<8>(buffer[1]) << "|" << std::bitset<8>(buffer[3]) << std::endl;
    // std::cout <<  << std::endl;

    // Reconstruct data
    *data = 0x00;
    *data = *data | (buffer[2] >> 6);
    *data = *data | (buffer[1] << 2);
    *data = *data | (buffer[0] << 10);

    // std::cout << std::bitset<16>(*data) << "|";

    return fcStatus;
}

FT_STATUS readManual(FT_HANDLE ftHandle)
{
    static FT_STATUS fcStatus;
    // double halfPeriod = (double)1/(freqHz*2.0);
    std::chrono::nanoseconds kHalfPeriod(5);

    // Set SCLK high before asserting CS
    uint8_t dir = 0x80;
    uint8_t val = 0x80;
    // fcStatus = FT_WriteGPIO(ftHandle, dir, val);
    // SPI_ToggleCS(ftHandle, TRUE);

    for(int i=0;i<30;i++)
    {
        // Falling edge SCLK
        FT_WriteGPIO(ftHandle, dir, 0x00);
        // std::this_thread::sleep_for(kHalfPeriod);

        // Rising edge SCLK
        FT_WriteGPIO(ftHandle, dir, 0x80);
        // std::this_thread::sleep_for(kHalfPeriod);
        if (i==13)
        {
           SPI_ToggleCS(ftHandle, TRUE); 
        }
    }

    SPI_ToggleCS(ftHandle, FALSE);

    return fcStatus;
}

std::string createUniqueFile() 
{
    // Debug
    std::string base_path = "../data/recordings";
    std::string extension = ".bin";
    int counter = 1;
    
    std::string file_path;
    
    // First try without any number
    file_path = base_path + extension;
    if (!fs::exists(file_path)) {
        std::cout << "Filename does not exit already... Opening " << file_path;
        return file_path;
    }
    
    // Then try with incrementing numbers
    do {
        file_path = base_path + std::to_string(counter) + extension;
        counter++;
    } while (fs::exists(file_path));
    std::cout << "Filename did exit already... Opening " << file_path <<"\n";

    return file_path;
}

int main(int argc, CHAR* argv[]) {
    printf("Buffer at beginning is %i bytes.\n", sizeof(sample_buffer));


    // Start writer thread 
    std::string uniqueName = createUniqueFile();
    std::ofstream storefile(uniqueName, std::ios::binary | std::ios::app);
    
    // -----------------------------------------------------------
    // Variables
    // -----------------------------------------------------------
    FT_HANDLE ftHandle; // Handle of the FTDI device
    FT_STATUS ftStatus; // Result of each D2XX call

    // -----------------------------------------------------------
    // Open the port - For this application note, we'll assume the first device is a
    // FT2232H or FT4232H. Further checks can be made against the device
    // descriptions, locations, serial numbers, etc. before opening the port.
    // -----------------------------------------------------------
    #ifdef _MSC_VER
        printf("MVSC detected: Initialising manually libMPSSE")
        Init_libMPSSE();
    #endif

    // -----------------------------------------------------------------
    // SPI Communication Set-up
    // -----------------------------------------------------------------
    // SPI Configuration
    channelConfSPI.ClockRate = 1000000;
    channelConfSPI.LatencyTimer = 255; // TODO: https://www.ftdichip.com/Support/Knowledgebase/index.html?settingacustomdefaultlaten.htm#:~:text=The%20latency%20timer%20is%20a,would%20not%20send%20data%20back.
    channelConfSPI.configOptions = \
    SPI_CONFIG_OPTION_MODE3 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channelConfSPI.Pin = 0x00000000;
    printf("MODE 3\n");
    // SPI Channel detection and opening
    // Get number of SPI channels
    ftStatus = SPI_GetNumChannels(&channels);
    APP_CHECK_STATUS(ftStatus);
    printf("There are %u SPI channels available.\n", channels);

    // Info about opened channel
    FT_DEVICE_LIST_INFO_NODE infoChanSPI;
    // ftStatus = SPI_GetChannelInfo(CHANNEL_TO_OPEN, &infoChanSPI);
    // printf("The opened channel has the followng properties:\n");
    // printf(" Flags=0x%x\n",infoChanSPI.Flags);
    // printf(" Type=0x%x\n",infoChanSPI.Type);
    // printf(" ID=0x%x\n",infoChanSPI.ID);
    // printf(" LocId=0x%x\n",infoChanSPI.LocId);
    // printf(" SerialNumber=%s\n",infoChanSPI.SerialNumber);
    // printf(" Description=%s\n",infoChanSPI.Description);
    // printf(" ftHandle=0x%x\n",infoChanSPI.ftHandle);
    ftStatus = SPI_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
    APP_CHECK_STATUS(ftStatus);
    printf("\nhandle=0x%x status=0x%x\n",ftHandle,ftStatus);
    ftStatus = SPI_InitChannel(ftHandle,&channelConfSPI);
    APP_CHECK_STATUS(ftStatus);

    // -----------------------------------------------------------
    // CORE 
    // -----------------------------------------------------------
    uint16_t data;
    size_t buffer_pos = 0;
    constexpr std::chrono::microseconds kSamplePeriod(250);
    auto next_sample_time = std::chrono::high_resolution_clock::now();

    for (int i=0; i<8000; i++) 
    {
        // Prepare next sampling time
        next_sample_time += kSamplePeriod;

        // Read and process
        readSample(ftHandle, &data);
        // printf("%u \n", data);
        // std::cout << (int16_t)data << std::endl;
        sample_buffer[buffer_pos++] = data;
        // int halfPeriod= 500;
        // readManual(ftHandle);

        // // 
        if (buffer_pos >= BUFFER_SIZE)
        {
            buffer_pos = 0;
            if (storefile.is_open())
            {
                std::cout << "Appending to storage file..." << std::endl;
                // storefile.rdbuf() -> pubsetbuf(buffer.data(), BUFFER_SIZE);
                printf("Buffer is %u bytes.", sizeof(sample_buffer));
                storefile.write((char*)&sample_buffer, sizeof(sample_buffer));
            } else {
                std::cerr << "Unable to add data." << std::endl;
            }
        } 

        // Wait until next sampling point
        std::this_thread::sleep_until(next_sample_time);
    }

    // ----------------------------------------------------------
    // Close and clean channel
    // ----------------------------------------------------------
    #ifdef _MSC_VER
        Cleanup_libMPSSE();
    #endif

    return 0; // Exit with success
}