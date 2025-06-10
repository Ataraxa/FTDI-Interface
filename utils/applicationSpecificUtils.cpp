#include <chrono>
#include <cstdint>
#include <math.h> 

using namespace std;

// Global variables
double Vref = 5.0;
double bitResDAC = 16.0;

uint16_t convertVoltToWord(int targetVoltage)
{
    uint16_t word;

    word = (targetVoltage+Vref)* pow(2.0, bitResDAC)/(2*Vref);

    return word;
}

