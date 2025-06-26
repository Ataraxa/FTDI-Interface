#include <vector>
#include <thread>
#include <functional>

#include "../include/thread_adbs.h"
#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"
#include "../include/dummy_utils.h"
#include "../include/SPI_utils.h"

void thread_adbs(ThreadSafeBuffer& memoryBuffer, SharedConfig& sharedCfg)
{
    // ------------------------------------------------------------------
    //    Shared variables
    // ------------------------------------------------------------------
    DWORD bytesToRead;
    std::function< int16_t(FT_HANDLE, uint16_t*) > readFunction;
    std::function< FT_STATUS(FT_HANDLE,LPVOID,DWORD,LPDWORD) > writeFunction;
    FT_STATUS ftStatus;

    // ------------------------------------------------------------------
    //    ADC Configuration
    // ------------------------------------------------------------------
    FT_HANDLE handleADC;
    ChannelConfig channelConfADC;

    channelConfADC.ClockRate = 1000000; // 1MHz SCLK
    channelConfADC.LatencyTimer = 255;
    channelConfADC.configOptions = \
    SPI_CONFIG_OPTION_MODE3 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channelConfADC.Pin = 0x00000000;

    #ifndef DEBUG
        FT_Open(0, &handleADC);
        configurePortMPSSE(&handleADC, bytesToRead);
        readFunction = readSample;
    #else 
        readFunction = readDummy;
    #endif

    // Variables related to input buffer
    uint16_t data;

    // ------------------------------------------------------------------
    //    DAC Configuration
    // ------------------------------------------------------------------
    FT_HANDLE handleDAC;
    int amplitude = 1; // in V
  
    #ifndef DEBUG
        FT_Open(1, &handleDAC);
        configurePortMPSSE(&handleDAC, bytesToRead);
        writeFunction = FT_Write;
    #else
        writeFunction = DUMMY_Write;
    #endif
    
    // Variables related to MPSSE buffer
    DWORD written;
    DWORD bufferLoc;
    BYTE buffer[2048];

    // ------------------------------------------------------------------
    //    Stimulation initialisation
    // ------------------------------------------------------------------
    // Amplitude
    uint16_t highWord;
    uint16_t lowWord;
    volt2word(amplitude, highWord, lowWord);    
    
    constexpr std::chrono::microseconds sampling_period(250);
    int16_t widthHighPulse = 100; // in µs
    int16_t widthInterPulse = 100; // in µs
    int16_t widthLowPulse = 100; // in µs
    auto next_sample_time = std::chrono::high_resolution_clock::now();

    // ------------------------------------------------------------------
    //    Syncronisation-related var.
    // ------------------------------------------------------------------
    int sampleClockCounter = 0;
    int sampleToStimDivider = 10;

    // ------------------------------------------------------------------
    //    MAIN LOOP
    // ------------------------------------------------------------------
    for(int i=0;i<10;i++)
    {
        next_sample_time += sampling_period;
        ftStatus = readFunction(handleADC, &data);
        sampleClockCounter++;
        memoryBuffer.addData(data);

        if(sampleClockCounter == sampleToStimDivider)
        {
            sampleClockCounter = 0;

            // Then need to send stimulus
            parseBuffer(buffer, bufferLoc, 
                widthHighPulse, widthInterPulse, widthLowPulse,
                highWord, lowWord
            );
            writeFunction(handleDAC, buffer, bufferLoc, &written);
        }   
        
        std::this_thread::sleep_until(next_sample_time);
    }
}
