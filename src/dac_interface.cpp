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

// Application specific macros
#define CHANNEL_TO_OPEN 0
#define SPI_DEVICE_BUFFER_SIZE 256
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};

namespace fs = std::filesystem;

// Global vars
ChannelConfig channelConfSPI;
DWORD channels;
uint8_t buffer[SPI_DEVICE_BUFFER_SIZE];
constexpr size_t BUFFER_SIZE = 4000;
std::array<uint16_t, BUFFER_SIZE> sample_buffer{}; 

