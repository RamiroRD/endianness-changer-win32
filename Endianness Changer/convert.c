#include <WinBase.h>
#include "stdafx.h"
#include "convert.h"
#include "math.h"


// TODO: Use hard coded conversions for 2, 4, 8 and 16 word sizes.
static void convertWord(char * word_ptr, int word_size)
{
    int i;
    char aux;
    for (i = 0; i<word_size / 2; i++)
    {
        aux = word_ptr[word_size - i - 1];
        word_ptr[word_size - i - 1] = word_ptr[i];
        word_ptr[i] = aux;
    }
}


// This receives an already opened handle and an allocated buffer
// Although itd be much better if we could use directly from the stack.
// I dont know if this is supported in whatever standard we're using (C99 I think)
// TODO: check if we can just use the stack
static DWORD64 _convertFiles(HANDLE src,
    HANDLE dest,
    const UCHAR word_size,
    char * buffer,
    WORD * progress,
    BOOL * cancel)
{
    DWORD64 totalRead = 0;
    DWORD64 totalWritten = 0;
    DWORD bytesRead;
    DWORD bytesWritten;
    LARGE_INTEGER size;
    if (!GetFileSizeEx(src, &size)
        || (SetFilePointer(src, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER))
    {   
        return 0;
    }

    for (DWORD64 i = 0; i<(DWORD64)size.QuadPart / word_size; i++)
    {
        if (*cancel)
            break;

        if (ReadFile(src, buffer, word_size, &bytesRead, NULL) == FALSE)
            break;
        totalRead += bytesRead;

        convertWord(buffer, word_size);
        if (dest == NULL)
        {
            if (SetFilePointer(src, -word_size, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                break;
            if (WriteFile(src, buffer, word_size, &bytesWritten, NULL) == FALSE)
                break;
        }
        else {
            if (WriteFile(dest, buffer, word_size, &bytesWritten, NULL) == FALSE)
                break;
        }
        totalWritten += bytesWritten;
        if (progress)
            *progress = 100.0 * ((float)totalWritten / size.QuadPart);
    }
    return totalWritten;
}

BOOL isPowerOfTwo(UCHAR x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

BOOL convertFiles(CONVERTARGS * args)
{
    if (!args)
        return FALSE;
    // Check if word_size is a power of 2.
    if (!isPowerOfTwo(args->wordSize))
        return FALSE;

    char * buffer = HeapAlloc(GetProcessHeap(),
        HEAP_GENERATE_EXCEPTIONS,
        args->wordSize);

    if (buffer == NULL)
        return FALSE;

    if (args->src == NULL)
        return FALSE;
    
    LARGE_INTEGER size;
    if (!GetFileSizeEx(args->src, &size))
        return FALSE;

    DWORD64 result = _convertFiles(args->src, args->des, args->wordSize, buffer, &(args->progress), &(args->cancel));
    BOOL success;
    if (args->bytesWritten)
        args->bytesWritten = result;

    if (size.QuadPart == result)
        success = TRUE;
    else
        success = FALSE;

    HeapFree(GetProcessHeap(), 0, buffer);
    return success;
}