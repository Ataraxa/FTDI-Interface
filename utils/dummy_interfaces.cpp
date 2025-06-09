#include <cstdio>
#include <random>
#include <thread>
#include "../include/dummy_interfaces.h"

void dummyPrint()
{
    printf("Hello from utils.");
}

uint32_t dummyReadADC()
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, (1 << 18) - 1);
    return dist(gen);
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
