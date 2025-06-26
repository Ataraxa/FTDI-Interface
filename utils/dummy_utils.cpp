#define _USE_MATH_DEFINES

#include <cstdio>
#include <random>
#include <thread>
#include <chrono>
#include <math.h>

#include "../include/dummy_utils.h"
#include "../include/ftd2xx.h"


FT_STATUS readDummy(FT_HANDLE ftHandle, uint16_t* data)
{   
    double voltage;
    uint16_t outputCode;
    
    // Time in seconds since program start
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - start;
    double t = elapsed.count(); // t in seconds
    
    // Parameter of the simulated sine wave
    int f = 20;
    
    // For noise generation
    static std::mt19937 gen(std::random_device{}());
    static std::normal_distribution<double> dist(0.0, 1.0); // Mean = 0, Std dev = 1
    double noiseSampleWGN = dist(gen);
    
    // Core of function
    voltage = 5 * std::sin(2 * 3.1415 * f * t) + noiseSampleWGN;
    outputCode = std::round(voltage/5*std::pow(2, 15));
    *data = outputCode;
    
    return FT_OK;
}

FT_STATUS DUMMY_Write(FT_HANDLE ftHandle,
        LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpdwBytesWritten)
{
    static FT_STATUS ftStatus = FT_OK;
    return ftStatus;
}

void dummyLongOperation()
{
    static std::chrono::milliseconds operationTime(500);
    std::this_thread::sleep_for(operationTime); 
}

int randomInt() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(-6, 6);
    return dist(gen);
}
