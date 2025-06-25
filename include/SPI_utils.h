#ifndef OPEN_SPI_H
#define OPEN_SPI_H
#pragma once

#include <cstdint>
#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

// ------------------------------------------------------------------
//    Channel opening and setup
// ------------------------------------------------------------------
int32_t setupPins(bool switch1, bool switch2);
FT_HANDLE openChannelSPI(ChannelConfig config, DWORD channels, int channelToOpen) ;
void configurePortMPSSE(FT_HANDLE *ftHandle, DWORD bytesToRead);

// ------------------------------------------------------------------
//    Read and Write Interface
// ------------------------------------------------------------------

FT_STATUS readSample(FT_HANDLE ftHandle, uint16_t *data);
void parseBuffer(BYTE* buffer, DWORD &bufferLoc,  
    uint16_t waitTime1,  uint16_t waitTime2,  uint16_t waitTime3, 
    uint16_t codeHigh, uint16_t codeLow);

// ------------------------------------------------------------------
//    Auxiliary Function
// ------------------------------------------------------------------

void volt2word(int amplitude, uint16_t& highWord, uint16_t& lowWord);

// Pre-allocated transaction buffer (packed for USB alignment)
#pragma pack(push, 1)
struct SPI_Transaction {
    uint8_t cs_low[3]  = {0x80, 0x00, 0x0B};  // GPIO: CS low (pin 0)
    uint8_t send_cmd[3] = {0x11, 0x02, 0x00};       // Send 2 bytes (command + 1st data)
    uint8_t data_byte[2];                         // 2nd data byte (updated per transfer)
    uint8_t cs_high[3] = {0x80, 0x08, 0x0B};  // GPIO: CS high
};
#pragma pack(pop)

#endif