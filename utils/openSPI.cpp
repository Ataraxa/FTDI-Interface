#include <cstdint>
#include <bitset>
#include <iostream>
#include <cstdio>

#include "../include/openSPI.h"
#include "../include/ftd2xx.h"
#include "../include/libmpsse_spi.h"

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};


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

FT_STATUS writeWord(FT_HANDLE ftHandle, uint16_t word, uint32_t transferOptions)
{
    // std::cout << std::bitset<16>(word) << std::endl;
    static FT_STATUS fcStatus;
    DWORD sizeTransferred = 0;
    uint32_t sizeToTransfer = 2;

    UCHAR bytes[2] = {
        static_cast<UCHAR>((word >> 8) & 0xFF),  // High byte
        static_cast<UCHAR>(word & 0xFF)          // Low byte
    };

    fcStatus = SPI_Write(ftHandle,bytes,sizeToTransfer,&sizeTransferred,transferOptions);
    // printf("Successfully transferred %u bits\n", sizeTransferred);
    return fcStatus;
}

// Pre-allocated buffer (global or thread-local for reuse)
// 0x80: GPIO write-> (1) Value | (2) Direction, 1 for out, 0 for in
// 0x11: clock out data -> (1) LSB length | (2) MSB length 


// Opti transfer (2 bytes only)
void SPI_Transfer_2Bytes(FT_HANDLE ftHandle, uint8_t byte1, uint8_t byte2) {
    thread_local SPI_Transaction txn;  // Reused buffer
    
    // Update transaction data (avoids alloc)
    txn.data_byte[0] = byte1;
    txn.data_byte[0] = byte2;  // 1st byte   // 2nd byte
    
    // Single USB write (7 bytes total)
    DWORD written;
    std::cout << bitset<>(txn) << std::endl
    FT_Write(ftHandle, &txn, sizeof(SPI_Transaction), &written);
}
