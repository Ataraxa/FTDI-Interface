#include <cstdio>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <iostream>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

using namespace std::chrono;

// Application-specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256

// Global variables
ChannelConfig channeConfSPI;
uint32_t channels;
uint8_t buffer[SPI_DEVICE_BUFFER_SIZE];

int BUFFER_SIZE = 4000;
std::vector<uint16_t> sample_buffer(BUFFER_SIZE);

void readDummySample(FT_HANDLE ftHandle, uint16_t *data)
{
    // Instantiate auxiliary variables
    static uint32_t sizeToTransfer = 0;
    static uint32_t sizeTransferred = 0; 

    // Read 16 bits from ADC
    sizeToTransfer = 16;
    sizeTransferred = 0;
    buffer[0] = 0x01;
    buffer[1] = 0xFE;
    // buffer[0] = rand() % 256;
    // buffer[1] = rand() % 256;

    // Reconstruct data
    *data = (uint16_t)(buffer[1]<<8);
    *data = (*data & 0xFF00) | (0x00FF & (uint16_t)buffer[0]);
}

int main(int argc, CHAR* argv[]) {

    // Start writer thread 
    std::ofstream storefile("recordings.bin", std::ios::binary | std::ios::app);
    
    // -----------------------------------------------------------
    // Variables
    // -----------------------------------------------------------
    FT_HANDLE ftHandle; // Handle of the FTDI device
    FT_STATUS ftStatus; // Result of each D2XX call

    // -----------------------------------------------------------------
    // SPI Communication Set-up
    // -----------------------------------------------------------------
    // SPI Configuration
    channeConfSPI.ClockRate = 5000;
    channeConfSPI.LatencyTimer = 255; // TODO: https://www.ftdichip.com/Support/Knowledgebase/index.html?settingacustomdefaultlaten.htm#:~:text=The%20latency%20timer%20is%20a,would%20not%20send%20data%20back.
    channeConfSPI.configOptions = \
    SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS5 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channeConfSPI.Pin = 0x00000000;

    // -----------------------------------------------------------
    // CORE 
    // -----------------------------------------------------------
    uint16_t data;
    size_t buffer_pos = 0;
    constexpr microseconds kSamplePeriod(250);
    auto next_sample_time = high_resolution_clock::now();

    auto start_write = high_resolution_clock::now();
    auto stop_write = high_resolution_clock::now();
    auto sampling1 = high_resolution_clock::now();
    auto sampling2 = high_resolution_clock::now();
    auto loopDuration = duration_cast<microseconds>(sampling1 - sampling2);
    auto writeDuration = duration_cast<microseconds>(stop_write - start_write);
    
    for (int i=0; i<10000; i++) 
    {
        // Prepare next sampling time
        sampling2 = high_resolution_clock::now(); 
        auto loopDuration = duration_cast<microseconds>(sampling2 - sampling1);
        sampling1 = high_resolution_clock::now(); 
        // std::cout << "Sampling period is" << loopDuration.count() << "us!\n";
        next_sample_time += kSamplePeriod;

        // Read and process
        readDummySample(ftHandle, &data);
        sample_buffer[buffer_pos++] = data;

        // 
        if (buffer_pos >= BUFFER_SIZE)
        {
            start_write = high_resolution_clock::now(); 
            buffer_pos = 0;
            if (storefile.is_open())
            {
                // storefile.rdbuf() -> pubsetbuf(buffer.data(), BUFFER_SIZE);
                storefile.write((char*)&data, sizeof(data));
            } else {
                std::cerr << "Unable to add data.";
            }
            stop_write = high_resolution_clock::now(); 
            writeDuration = duration_cast<microseconds>(stop_write - start_write);
            std::cout << "It took " << writeDuration.count() <<" us to write!\n";
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
    printf("Arbeit vertigen");
    return 0; // Exit with success
}