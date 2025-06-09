#include <cstdint>
#include <bitset>
#include <iostream>
#include <cstdio>

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

FT_STATUS writeWord(FT_HANDLE ftHandle, uint16_t word)
{
    static FT_STATUS fcStatus;
    DWORD sizeTransferred = 0;
    uint32_t sizeToTransfer = 16;
    uint32_t transferOptions = SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE | SPI_TRANSFER_OPTIONS_SIZE_IN_BITS;

    UCHAR bytes[2] = {
        static_cast<UCHAR>((word >> 8) & 0xFF),  // High byte
        static_cast<UCHAR>(word & 0xFF)          // Low byte
    };

    fcStatus = SPI_Write(ftHandle,bytes,sizeToTransfer,&sizeTransferred,transferOptions);
    // printf("Successfully transferred %u bits\n", sizeTransferred);
    return fcStatus;
}
