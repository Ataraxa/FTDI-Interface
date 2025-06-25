#include<vector>
#include <thread>

#include "../include/thread_adbs.h"
#include "../include/threadSafeBuffer.h"
#include "../include/sharedConfig.h"
#include "../include/dummy_interfaces.h"
#include "../include/SPI_utils.h"

void thread_adbs(ThreadSafeBuffer& memoryBuffer, SharedConfig& sharedCfg) {
    // ------------------------------------------------------------------
    //    Shared variables
    // ------------------------------------------------------------------
    DWORD bytesToRead;

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

    FT_Open(0, &handleADC);
    configurePortMPSSE(&handleADC, bytesToRead);

    // Variables related to input buffer
    uint16_t data;

    // ------------------------------------------------------------------
    //    DAC Configuration
    // ------------------------------------------------------------------
    FT_HANDLE handleDAC;

    // std::vector<int> config;
    // config = sharedCfg.getCurrent();
    // int frequency = config[0];
    // int amplitude = config[1];
    int amplitude = 1; // in V
  
    FT_Open(1, &handleDAC);
    configurePortMPSSE(&handleDAC, bytesToRead);

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
    for(int i=0;i<1000;i++)
    {
        next_sample_time += sampling_period;
        data = dummyReadADC();
        sampleClockCounter++;

        if(sampleClockCounter=sampleToStimDivider)
        {
            // Then need to send stimulus
            parseBuffer(buffer, bufferLoc, 
                widthHighPulse, widthInterPulse, widthLowPulse,
                highWord, lowWord
            );
            FT_Write(handleDAC, buffer, bufferLoc, &written);
        }   
        
        std::this_thread::sleep_until(next_sample_time);
    }
}
