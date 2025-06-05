#include "../include/ftd2xx.h"
#include "../include/optiRead.h"

FT_STATUS SPI_optiRead(FT_HANDLE handle, UCHAR *buffer,
	DWORD sizeToTransfer, LPDWORD sizeTransferred, DWORD transferOptions)
{
	FT_STATUS status;
	//uint32 i;
	uint8 byte;
	uint8 bitsToTransfer = 0;
	uint8 lsb = 0;

	
	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(handle);
	CHECK_NULL_RET(buffer);
	CHECK_NULL_RET(sizeTransferred);
#endif
	LOCK_CHANNEL(handle);

    // ADS161S IS MSB FIRST ANYWAY!
	// if (transferOptions & SPI_TRANSFER_OPTIONS_LSB_FIRST)
	// {
	// 	lsb = MPSSE_CMD_DATA_LSB_FIRST;
	// }
	
    status = SPI_ToggleCS(handle, TRUE);
    CHECK_STATUS(status);


    *sizeTransferred = 0;
    while(*sizeTransferred < sizeToTransfer)
    {
        if ((sizeToTransfer - *sizeTransferred)>=8)
            bitsToTransfer = 8;
        else
            bitsToTransfer = (UCHAR)(sizeToTransfer - *sizeTransferred);
        status = SPI_ReadADC161S(handle, &byte, bitsToTransfer, lsb);
        buffer[(*sizeTransferred+1)/8] = byte;
        CHECK_STATUS(status);
        if (FT_OK == status)
            *sizeTransferred += bitsToTransfer;
    }
	

	if (transferOptions & SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE)
	{
		/* Disable CHIPSELECT line for the channel */
		status = SPI_ToggleCS(handle, FALSE);
		CHECK_STATUS(status);
	}
	UNLOCK_CHANNEL(handle);
	DBG(MSG_DEBUG,"sizeToTransfer=%u  sizeTransferred=%u BitMode=%u \
		CS_Enable=%u CS_Disable=%u\n", sizeToTransfer,*sizeTransferred,
		(unsigned)(transferOptions & SPI_TRANSFER_OPTIONS_SIZE_IN_BITS),
		(unsigned)(transferOptions & SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE),
		(unsigned)(transferOptions & SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE));
	FN_EXIT;
	return status;
}

static FT_STATUS SPI_ReadADC161S(FT_HANDLE handle, uint8 *byte, uint8 len, uint8 lsb)
{
	FT_STATUS status = FT_OTHER_ERROR;
	DWORD noOfBytes = 0, noOfBytesTransferred = 0;
	uint8 buffer[10];
	ChannelConfig *config = NULL;
	uint8 mode;

	FN_ENTER;
	status = SPI_GetChannelConfig(handle, &config);
	CHECK_STATUS(status);
	/*mode is given by bit1-bit0 of ChannelConfig.Options*/
	mode = (config->configOptions & SPI_CONFIG_OPTION_MODE_MASK);
	/* Command to write 8bits */
	switch(mode)
	{
		case 0:
			buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE | lsb;
			break;
		case 1:
			buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_NEG_EDGE | lsb;
			break;
		case 2:
			buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_NEG_EDGE | lsb;
			break;
		case 3:
			buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE | lsb;
			break;
		default:
			DBG(MSG_ERR,"invalid mode(%u)\n",(unsigned)mode);
	}
	buffer[noOfBytes++] = len-1;/* 1bit->arg = 0, for 8bits->arg = 7 */

	/*Command MPSSE to send data to PC immediately */
	buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;

	DBG(MSG_DEBUG,"writing data = 0x%x len=%u\n",(unsigned)byte,(unsigned)len);
	status = FT_Channel_Write(SPI, handle, noOfBytes, buffer,\
		&noOfBytesTransferred);
	CHECK_STATUS(status);

	noOfBytes = 1;
	noOfBytesTransferred = 0;
	status = FT_Channel_Read(SPI, handle, noOfBytes, buffer, &noOfBytesTransferred);
	CHECK_STATUS(status);

	*byte = buffer[0];
	DBG(MSG_DEBUG,"SPI_Read8bits len=%u(in bits) byte = 0x%x mode=%u\n",\
		(unsigned)len,*byte,(unsigned)mode);

	FN_EXIT;
	return status;
}
