#pragma once

#include "stdafx.h"

typedef struct
{
    HANDLE src;
    HANDLE des;
    DWORD64 bytesWritten;
    volatile WORD progress;
    WORD blockSize;
    UCHAR wordSize;
    volatile UCHAR threads;
    BOOL cancel;
} CONVERTARGS;

BOOL convertFiles(CONVERTARGS * args);