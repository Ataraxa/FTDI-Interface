#ifndef DUMMY_INTERFACE_H
#define DUMMY_INTERFACE_H

#include "../include/ftd2xx.h"

FT_STATUS readDummy(FT_HANDLE ftHandle, uint16_t* data);

FT_STATUS DUMMY_Write(FT_HANDLE ftHandle,
        LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpdwBytesWritten);
#endif