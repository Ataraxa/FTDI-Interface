/*!
* \file sample-static.c
*
* \author FTDI
* \date 20131002
*
* Copyright © 2013 Future Technology Devices International Limited
* Company Confidential
*
* Project: libMPSSE
* Module: SPI Sample Application - Interfacing 94LC56B SPI EEPROM
*
* Rivision History:
* 0.1 - 20110512 - Initial version
* 0.2 - 20110801 - Changed LatencyTimer to 255
* Attempt to open channel only if available
* Added & modified macros
* Included stdlib.h
* 0.3 - 20111212 - Added comments
* 0.4 - 20131002 - Added comments
*/
/******************************************************************************/
/* Include files
 */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
/* OS specific libraries */
#ifdef _WIN32
#include<windows.h>
#endif
/* Include D2XX header*/
#include "ftd2xx.h"
/* Include libMPSSE header */
#include "libMPSSE_spi.h"
/******************************************************************************/
/* Macro and type defines
 */
/******************************************************************************/
/* Helper macros */
#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s(): NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};
/* Application specific macro definations */
#define SPI_DEVICE_BUFFER_SIZE 256
#define SPI_WRITE_COMPLETION_RETRY 10
#define START_ADDRESS_EEPROM 0x00 /*read/write start address inside the EEPROM*/
#define END_ADDRESS_EEPROM 0x10
#define RETRY_COUNT_EEPROM 10 /* number of retries if read/write fails */
#define CHANNEL_TO_OPEN 0 /*0 for first available channel, 1 for next... */
#define SPI_SLAVE_0 0
#define SPI_SLAVE_1 1
#define SPI_SLAVE_2 2
#define DATA_OFFSET 3
/******************************************************************************/
/* Global variables
 */
/******************************************************************************/
uint32 channels;
FT_HANDLE ftHandle;
ChannelConfig channelConf;
uint8 buffer[SPI_DEVICE_BUFFER_SIZE];
/******************************************************************************/
/* Public function definitions
 */
/******************************************************************************/
/*!
* \brief Writes to EEPROM
*
* This function writes a byte to a specified address within the 93LC56B EEPROM
*
* \param[in] slaveAddress Address of the I2C slave (EEPROM)
* \param[in] registerAddress Address of the memory location inside the slave to where the byte
* is to be written
* \param[in] data The byte that is to be written
* \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
* \sa Datasheet of 93LC56B http://ww1.microchip.com/downloads/en/DeviceDoc/21794F.pdf
* \note
* \warning
*/
FT_STATUS read_byte(uint8 slaveAddress, uint8 address, uint16 *data)
{
    uint32 sizeToTransfer = 0;
    uint32 sizeTransfered;
    bool writeComplete=0;
    uint32 retry=0;
    bool state;
    FT_STATUS status;
    
    /* CS_High + Write command + Address */
    sizeToTransfer=1;
    sizeTransfered=0;
    buffer[0] = 0xC0;/* Write command (3bits)*/
    buffer[0] = buffer[0] | ( ( address >> 3) & 0x0F );/*5 most significant add bits*/
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    APP_CHECK_STATUS(status);
    /*Write partial address bits */
    sizeToTransfer=4;
    sizeTransfered=0;
    buffer[0] = ( address & 0x07 ) << 5; /* least significant 3 address bits */
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
    APP_CHECK_STATUS(status);
    /*Read 2 bytes*/
    sizeToTransfer=2;
    sizeTransfered=0;
    status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);
    *data = (uint16)(buffer[1]<<8);
    *data = (*data & 0xFF00) | (0x00FF & (uint16)buffer[0]);

    return status;
}
/*!
* \brief Reads from EEPROM
*
* This function reads a byte from a specified address within the 93LC56B EEPROM
*
* \param[in] slaveAddress Address of the I2C slave (EEPROM)
* \param[in] registerAddress Address of the memory location inside the slave from where the
* byte is to be read
* \param[in] *data Address to where the byte is to be read
* \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
* \sa Datasheet of 93LC56B http://ww1.microchip.com/downloads/en/DeviceDoc/21794F.pdf
* \note
* \warning
*/
FT_STATUS write_byte(uint8 slaveAddress, uint8 address, uint16 data)
{
    uint32 sizeToTransfer = 0;
    uint32 sizeTransfered=0;
    bool writeComplete=0;
    uint32 retry=0;
    bool state;
    FT_STATUS status;
    /* Write command EWEN(with CS_High -> CS_Low) */
    sizeToTransfer=11;
    sizeTransfered=0;
    buffer[0]=0x9F;/* SPI_EWEN -> binary 10011xxxxxx (11bits) */
    buffer[1]=0xFF;
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);
    /* CS_High + Write command + Address */
    sizeToTransfer=1;
    sizeTransfered=0;
    buffer[0] = 0xA0;/* Write command (3bits) */
    buffer[0] = buffer[0] | ( ( address >> 3) & 0x0F );/*5 most significant add bits*/
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    APP_CHECK_STATUS(status);
    /*Write 3 least sig address bits */
    sizeToTransfer=3;
    sizeTransfered=0;
    buffer[0] = ( address & 0x07 ) << 5; /* least significant 3 address bits */
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
    APP_CHECK_STATUS(status);
    /* Write 2 byte data + CS_Low */
    sizeToTransfer=2;
    sizeTransfered=0;
    buffer[0] = (uint8)(data & 0xFF);
    buffer[1] = (uint8)((data & 0xFF00)>>8);
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);
    /* Wait until D0 is high */
    #if 1
    /* Strobe Chip Select */
    sizeToTransfer=0;
    sizeTransfered=0;
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    APP_CHECK_STATUS(status);
    #ifndef __linux__
    Sleep(10);
    #endif
    sizeToTransfer=0;
    sizeTransfered=0;
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);
    #else
    retry=0;
    state=FALSE;
    SPI_IsBusy(ftHandle,&state);
    while((FALSE==state) && (retry<SPI_WRITE_COMPLETION_RETRY))
    {
    printf("SPI device is busy(%u)\n",(unsigned)retry);
    SPI_IsBusy(ftHandle,&state);
    retry++;
    }
    #endif
    /* Write command EWEN(with CS_High -> CS_Low) */
    sizeToTransfer=11;
    sizeTransfered=0;
    buffer[0]=0x8F;/* SPI_EWEN -> binary 10011xxxxxx (11bits) */
    buffer[1]=0xFF;
    status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
    SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    APP_CHECK_STATUS(status);
    return status;
}
/*!
* \brief Main function / Entry point to the sample application
*
* This function is the entry point to the sample application. It opens the channel, writes to the
* EEPROM and reads back.
*
* \param[in] none
* \return Returns 0 for success
* \sa
* \note
* \warning
*/
int main()
{
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE devList;
    uint8 address=0;
    uint16 data;
    int i,j;

    #ifdef _MSC_VER
        Init_libMPSSE();
    #endif

    channelConf.ClockRate = 5000;
    channelConf.LatencyTimer= 255;
    channelConf.configOptions = \
    SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3;
    channelConf.Pin = 0x00000000;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/
    status = SPI_GetNumChannels(&channels);
    APP_CHECK_STATUS(status);

    printf("Number of available SPI channels = %d\n",channels);
    if(channels>0)
    {
        for(i=0;i<channels;i++)
        {
            status = SPI_GetChannelInfo(i,&devList);
            APP_CHECK_STATUS(status);
            printf("Information on channel number %d:\n",i);
            /* print the dev info */
            printf(" Flags=0x%x\n",devList.Flags);
            printf(" Type=0x%x\n",devList.Type);
            printf(" ID=0x%x\n",devList.ID);
            printf(" LocId=0x%x\n",devList.LocId);
            printf(" SerialNumber=%s\n",devList.SerialNumber);
            printf(" Description=%s\n",devList.Description);
            printf(" ftHandle=0x%x\n",devList.ftHandle);/*is 0 unless open*/
        }
        /* Open the first available channel */
        status = SPI_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
        APP_CHECK_STATUS(status);
        printf("\nhandle=0x%x status=0x%x\n",ftHandle,status);
        status = SPI_InitChannel(ftHandle,&channelConf);
        APP_CHECK_STATUS(status);
        for(address=START_ADDRESS_EEPROM;address<END_ADDRESS_EEPROM;address++)
        {
            printf("writing address = %d data = %d\n", address, \
            address+DATA_OFFSET);
            write_byte(SPI_SLAVE_0, address, (uint16)address+DATA_OFFSET);
        }
        for(address=START_ADDRESS_EEPROM;address<END_ADDRESS_EEPROM;address++)
        {
            read_byte(SPI_SLAVE_0, address,&data);
            read_byte(SPI_SLAVE_0, address,&data);
            printf("reading address=%d data=%d\n",address,data);
        }
        status = SPI_CloseChannel(ftHandle);
    }
    #ifdef _MSC_VER
    Cleanup_libMPSSE();
    #endif
    return 0;
}
