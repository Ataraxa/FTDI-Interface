#include <cstdint>
#include <bitset>
#include <iostream>
#include <cstdio>
#include <cmath>

#include "../include/SPI_utils.h"
#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};

// ------------------------------------------------------------------
//    Constants
// ------------------------------------------------------------------
int Vref = 5;

// ------------------------------------------------------------------
//    Channel opening and setup
// ------------------------------------------------------------------

int32_t setupPins(bool switch1, bool switch2)
{   
    uint32_t pins = 0x00000000;
    uint8_t dirMask = 0x30;
    uint8_t stateMask = (switch1 ? 0x20 : 0x00) | (switch2 ? 0x10 : 0x00);
    uint16_t unusedMask = 0x0000;
    pins = (static_cast<int32_t>(unusedMask) << 16) |
           (static_cast<int32_t>(stateMask) << 8)  |
           (static_cast<int8_t>(dirMask));
         
    std::cout << "Values written:" << std::bitset<32>(pins) << std::endl;

    return pins;
}

FT_HANDLE openChannelSPI(ChannelConfig config, DWORD channels, int channelToOpen) 
{
    static FT_STATUS ftStatus;
    FT_HANDLE ftHandle;

    // Get number of channels
    ftStatus = SPI_GetNumChannels(&channels);
    APP_CHECK_STATUS(ftStatus);
    printf("There are %u SPI channels available.\n", channels);

    // Info about channel of interest
    FT_DEVICE_LIST_INFO_NODE infoChanSPI;
    ftStatus = SPI_GetChannelInfo(channelToOpen, &infoChanSPI);
    printf("The opened channel has the followng properties:\n");
    printf(" Flags=0x%x\n",infoChanSPI.Flags);
    printf(" Type=0x%x\n",infoChanSPI.Type);
    printf(" ID=0x%x\n",infoChanSPI.ID);
    printf(" LocId=0x%x\n",infoChanSPI.LocId);
    printf(" SerialNumber=%s\n",infoChanSPI.SerialNumber);
    printf(" Description=%s\n",infoChanSPI.Description);
    printf(" ftHandle=0x%x\n",infoChanSPI.ftHandle);

    // Open channel
    ftStatus = SPI_OpenChannel(channelToOpen, &ftHandle);
    APP_CHECK_STATUS(ftStatus);
    printf("\nhandle=0x%x status=0x%x\n",ftHandle,ftStatus);
    ftStatus = SPI_InitChannel(ftHandle,&config);
    APP_CHECK_STATUS(ftStatus);

    return ftHandle;
}

void configurePortMPSSE(FT_HANDLE *ftHandle, DWORD bytesToRead)
{
    FT_STATUS ftStatus;
    printf("\nConfiguring port for MPSSE use...\n");
    ftStatus |= FT_ResetDevice(ftHandle);
    //Purge USB receive buffer first by reading out all old data from FT2232H receive buffer
    ftStatus |= FT_GetQueueStatus(ftHandle, &bytesToRead);
    ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65536); //Set USB request transfer sizes to 64K
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0); //Disable event and error characters
    ftStatus |= FT_SetTimeouts(ftHandle, 0, 5000); //Sets the read and write timeouts in milliseconds
    ftStatus |= FT_SetLatencyTimer(ftHandle, 1); //Set the latency timer to 1mS (default is 16mS)
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x00, 0x00);
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00); // Reset controller
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02); //Enable MPSSE mode
    APP_CHECK_STATUS(ftStatus)
}
// Pre-allocated buffer (global or thread-local for reuse)
// 0x80: GPIO write-> (1) Value | (2) Direction, 1 for out, 0 for in
// 0x11: clock out data -> (1) LSB length | (2) MSB length 

// ------------------------------------------------------------------
//    Read and Write Interface
// ------------------------------------------------------------------


FT_STATUS readSample(FT_HANDLE ftHandle, uint16_t* data)
{
    // Instantiate auxiliary variables
    static uint32_t sizeToTransfer = 0;
    static DWORD sizeTransferred = 0; 
    static FT_STATUS fcStatus;

    uint8_t buffer[3] = {};
    // std::cout << std::to_string(buffer[0]) << std::endl;
    // std::cout << std::to_string(buffer[1]) << std::endl;

    // Read 16 bits from ADC
    sizeToTransfer = 18;
    sizeTransferred = 0;
    fcStatus = SPI_Read(ftHandle, (UCHAR *)&buffer, sizeToTransfer, &sizeTransferred,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(fcStatus);

    // std::cout << std::bitset<8>(buffer[0]) << "|" << std::bitset<8>(buffer[1]) << "|" << std::bitset<8>(buffer[3]) << std::endl;
    // std::cout <<  << std::endl;

    // Check data validity
    if(buffer[0] & 0xC0 != 0x80) {
        // std::cout << "First buffer invalid: " << buffer[0] << std::endl;
    }

    // Reconstruct data
    *data = 0x0000;
    *data = *data | (buffer[2] >> 6);
    *data = *data | (buffer[1] << 2);
    *data = *data | (buffer[0] << 10);

    std::cout << std::bitset<16>(*data) << "|";

    return fcStatus;
}

void parseBuffer(BYTE* buffer,
    DWORD &bufferLoc, 
    uint16_t waitTime1, 
    uint16_t waitTime2, 
    uint16_t waitTime3,
    uint16_t codeHigh,
    uint16_t codeLow)
    {
    bufferLoc = 0;
    
    // High pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;
    
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = (codeHigh >> 8 ) & 0xFF;
    buffer[bufferLoc++] = codeHigh & 0xFF;
    
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;
    
    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime1 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime1;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }
    
    // Inter pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;
    
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x00;
    
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;
    
    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime2 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime2;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }
    
    // Low pulse
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;
    
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = (codeLow >> 8) & 0xFF;
    buffer[bufferLoc++] = codeLow & 0xFF;
    
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;
    
    // Waiting time
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = (uint16_t)(waitTime3 - 1);
    buffer[bufferLoc++] = 0x00;
    for(int i=0;i<waitTime3;i++)
    {
        buffer[bufferLoc++] = 0x00;
    }
    
    // Reset to null
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x48;
    buffer[bufferLoc++] = 0xFB;
    
    buffer[bufferLoc++] = 0x11;
    buffer[bufferLoc++] = 0x01;
    buffer[bufferLoc++] = 0x00;
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0x00;
    
    buffer[bufferLoc++] = 0x80;
    buffer[bufferLoc++] = 0xC8;
    buffer[bufferLoc++] = 0xFB;
}

// ------------------------------------------------------------------
//    Auxiliary Function
// ------------------------------------------------------------------

void volt2word(int amplitude, uint16_t& highWord, uint16_t& lowWord)
{
    highWord = std::round((amplitude+Vref)/(2*Vref)*std::pow(2, 16));
    lowWord = std::round((-amplitude+Vref)/(2*Vref)*std::pow(2, 16));
}
