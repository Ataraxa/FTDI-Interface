#include <cstdio>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <atomic>

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

// Application-specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256

// Global variables
ChannelConfig channeConfSPI;
uint32 channels;
uint8 buffer[SPI_DEVICE_BUFFER_SIZE];

constexpr size_t BUFFER_SIZE = 4000;
std::vector<uint16_t> sample_buffer(BUFFER_SIZE);
std::atomic<size_t> write_pos(0);

void writerThread(const std::string& filename)
{
    std::ofstream file(filenae, std::ios::binary | std::ios:trunc);
    if (!file) thros std::runtime_error("Failed to open file!");

    while (!stop_signal.load()) 
    {
        // Wait for new samples
        size_t current_pos = write_pos.load();
        if (current_pos == 0)
        {
            std::this_thread::yield();
            continue;
        }

        // Write all available samples
        file.write(reinterpret_cast<const char*>(sample_buffer.data()),
                 current_pos * sizeof(uint16_t));
        file.flush(); // Ensure data hits disk
        write_pos = 0;
    } 
}

FT_STATUS readSample(FT_HANDLE ftHandle, uint16 *data)
{
    // Instantiate auxiliary variables
    static uint32 sizeToTransfer = 0;
    static uint32 sizeTransferred = 0; 

    // Read 16 bits from ADC
    sizeToTransfer = 16;
    sizeTransferred = 0;
    ftStatus = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransferred,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS |
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(ftStatus);

    // Reconstruct data
    *data = (uint16)(buffer[1]<<8);
    *data = (*data & 0xFF00) | (0x00FF & (uint16)buffer[0]);

    return status;
}

int main(int argc, CHAR* argv[]) {

    // Start writer thread 
    std::thread writer(writer_thread, "samples.bin");
    
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
    channeConfSPI.ClockRate = 5000;
    channeConfSPI.LatencyTimer = 255; // TODO: https://www.ftdichip.com/Support/Knowledgebase/index.html?settingacustomdefaultlaten.htm#:~:text=The%20latency%20timer%20is%20a,would%20not%20send%20data%20back.
    channeConfSPI.configOptions = \
    SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS5 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channeConfSPI.Pin = 0x00000000;

    // SPI Channel detection and opening
    ftStatus = SPI_GetNumChannels(&channels);
    APP_CHECK_STATUS(ftStatus);
    ftStatus = SPI_OpenChannel(CHANNEL_TO_OPEN, &ftHandle);
    APP_CHECK_STATUS(ftStatus);
    printf("\nhandle=0x%x status=0x%x\n",ftHandle,status);
    status = SPI_InitChannel(ftHandle,&channelConf);
    APP_CHECK_STATUS(status);

    // -----------------------------------------------------------
    // CORE 
    // -----------------------------------------------------------
    uint16 data;
    size_t buffer_pos = 0;
    constexpr std:chrono::microseconds kSamplePeriod(250);
    auto next_sample_time = std::chrono::high_resolution_clock::now();

    for (int i=0; i<10; i++) 
    {
        // Prepare next sampling time
        next_sample_time += kSamplePeriod;

        // Read and process
        readSample(ftHandle, &data);
        sample_buffer[buffer_pos++] = data;

        // 
        if (buffer_pos >= BUFFER_SIZE)
        {
            write_pos.store(buffer_pos);
            buffer_pos = 0;
            while (write_pos.load() != 0)
            {
                std::this_thread::yield();
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