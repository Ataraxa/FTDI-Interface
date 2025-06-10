#ifndef OPEN_SPI_H
#define OPEN_SPI_H
#pragma once

#include <cstdint>
#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

void SPI_Transfer_2Bytes(FT_HANDLE ftHandle, uint8_t byte1, uint8_t byte2);

int32_t setupPins(bool switch1, bool switch2);

// Pre-allocated transaction buffer (packed for USB alignment)
#pragma pack(push, 1)
struct SPI_Transaction {
    uint8_t cs_low[3]  = {0x80, 0x00, 0x0B};  // GPIO: CS low (pin 0)
    uint8_t send_cmd[3] = {0x11, 0x02, 0x00};       // Send 2 bytes (command + 1st data)
    uint8_t data_byte[2];                         // 2nd data byte (updated per transfer)
    uint8_t cs_high[3] = {0x80, 0x08, 0x0B};  // GPIO: CS high
};
#pragma pack(pop)

FT_HANDLE openChannelSPI(ChannelConfig config, DWORD channels, int channelToOpen) ;

FT_STATUS writeWord(FT_HANDLE ftHandle, uint16_t word, uint32_t transferOptions);

#endif