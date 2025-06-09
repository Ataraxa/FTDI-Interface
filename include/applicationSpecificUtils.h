#ifndef CONFIG_DBS_H
#define CONFIG_DBS_H

#include <cstdint> 
#include <chrono> 

uint16_t convertVoltToWord(int targetVoltage);

struct ConfigDBS {
    int frequency;
    uint32_t lowAmplitudeWord;
    uint32_t highAmplitudeWord;
    uint32_t nullWord;
    std::chrono::microseconds highTime;
    std::chrono::microseconds interTime;
    std::chrono::microseconds lowTime;
    std::chrono::microseconds totalTime;
};

#endif // CONFIG_DBS_H
