#pragma once

#include "stdafx.h"

typedef struct
{
    HANDLE src;
    HANDLE des;
    UCHAR wordSize;
    DWORD64 bytesWritten;
    WORD progress;
    BOOL cancel;
    UCHAR threads;
    WORD blockSize;
} CONVERTARGS;

BOOL convertFiles(CONVERTARGS * args);