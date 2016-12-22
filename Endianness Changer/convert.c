#include <WinBase.h>
#include "stdafx.h"


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
static DWORD64 _convertFiles(HANDLE src, HANDLE dest, const UCHAR word_size, char * buffer)
{
    DWORD64 totalRead = 0;
    DWORD64 totalWritten = 0;
    DWORD bytesRead;
    DWORD bytesWritten;
    LARGE_INTEGER size;
    if (!GetFileSizeEx(src, &size))
        return 0;
    
    if (SetFilePointer(src, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return FALSE;

    for (DWORD64 i = 0; i<(DWORD64)size.QuadPart / word_size; i++)
    {
        if (ReadFile(src, buffer, word_size, &bytesRead, NULL) == FALSE)
            return totalWritten;
        totalRead += bytesRead;

        convertWord(buffer, word_size);
        if (dest == NULL)
        {
            if (SetFilePointer(src, -word_size, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                return totalWritten;
            if (WriteFile(src, buffer, word_size, &bytesWritten, NULL) == FALSE)
                return totalWritten;
        }
        else {
            if (WriteFile(dest, buffer, word_size, &bytesWritten, NULL) == FALSE)
                return totalWritten;
        }

    }
    return totalWritten;
}

BOOL isPowerOfTwo(UCHAR x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

BOOL convertFiles(HANDLE src, HANDLE des, UCHAR wordSize, DWORD64 * bytesWritten)
{
    // Check if word_size is a power of 2.
    if (!isPowerOfTwo(wordSize))
        return FALSE;

    char * buffer = HeapAlloc(GetProcessHeap(),
        HEAP_GENERATE_EXCEPTIONS,
        wordSize);

    if (buffer == NULL)
        return FALSE;

    if (src == NULL)
        return FALSE;
    
    LARGE_INTEGER size;
    if (!GetFileSizeEx(src, &size))
        return FALSE;

    DWORD64 result = _convertFiles(src, des, wordSize, buffer);
    BOOL success;
    if (bytesWritten)
        *bytesWritten = result;

    if (size.QuadPart == result)
        success = TRUE;
    else
        success = FALSE;

    HeapFree(GetProcessHeap(), 0, buffer);
    return success;
}