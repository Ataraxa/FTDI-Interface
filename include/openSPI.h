#ifndef OPEN_SPI_H
#define OPEN_SPI_H

#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

int32_t setupPins(bool switch1, bool switch2);

FT_HANDLE openChannelSPI(ChannelConfig config, DWORD channels, int channelToOpen) ;

FT_STATUS writeWord(FT_HANDLE ftHandle, uint16_t word);

#endif