#include <WinBase.h>
#include <math.h>
#include "stdafx.h"
#include "convert.h"


typedef struct
{
    HANDLE  inFile;
    HANDLE  outFile;
    DWORD64 readOffset;  /* next byte to read */
    DWORD64 writeOffset; /* next byte to write */
    HANDLE  readMutex;   /* protects readOffset and provides read exclusion */
    HANDLE  writeMutex;  /* protects writeOffset and provides write exclusion */
    WORD    blockSize;   /* amount of words to process at once */
    WORD    wordSize;
    DWORD64 totalWords;
    WORD    error;
} CONVERSIONSTRUCT;



static void convertWord(char * word_ptr, UCHAR word_size)
{
    int i;
    char aux;
    for (i = 0; i < word_size / 2; i++)
    {
        aux = word_ptr[word_size - i - 1];
        word_ptr[word_size - i - 1] = word_ptr[i];
        word_ptr[i] = aux;
    }
}

static void convertWord2(char * word_ptr)
{
    char aux = word_ptr[0];
    word_ptr[0] = word_ptr[1];
    word_ptr[1] = aux;
}

static void convertWord4(char * word_ptr)
{
    char aux = word_ptr[0];
    // outer
    word_ptr[0] = word_ptr[3];
    word_ptr[3] = aux;
    // inner
    aux = word_ptr[1];
    word_ptr[1] = word_ptr[2];
    word_ptr[2] = aux;
}

static void convertBlock(char * word_ptr, UCHAR word_size, WORD n)
{
    WORD i;
    for (i = 0; i < n; i++)
    {
        switch (word_size)
        {
        case 2:
            convertWord2(word_ptr + i*word_size);
            break;
        case 4:
            convertWord4(word_ptr + i*word_size);
            break;
        default:
            convertWord (word_ptr + i*word_size, word_size);
            break;
        } 
    }
}


static DWORD WINAPI processBlock(LPVOID args)
{
    CONVERSIONSTRUCT * context = (CONVERSIONSTRUCT*)args;
    DWORD64 readOffsetLocal;
    DWORD64 writeOffsetLocal;
    /* Should always be the same as the block size in bytes after every IO operation */
    DWORD   bytesIO;
    /* We do not check for NULL, an exception will be thrown if allocation fails */
    BYTE * buffer = HeapAlloc(GetProcessHeap(),
                    HEAP_GENERATE_EXCEPTIONS,
                    context->wordSize);
    DWORD blockSizeBytes = context->wordSize * context->blockSize;
    
    /* This loop will be executed as long as there are bytes left to read and there is no error */
    while(!context->error)
    {
        /* We find out the offset of the next block to read, process and write */
        if (WaitForSingleObject(context->readMutex, INFINITE) != WAIT_OBJECT_0)
        {
            context->error = TRUE;
            break;
        }
        readOffsetLocal = context->readOffset;

        /* Have all the words already been read? */
        if (readOffsetLocal == blockSizeBytes)
        {
            /* We are done, release the lock and exit */
            ReleaseMutex(context->readMutex);
            break;
        }
        
        /* For last block read, reduce block size */
        if (context->totalWords * context->wordSize - readOffsetLocal < blockSizeBytes)
            blockSizeBytes = context->totalWords * context->wordSize - readOffsetLocal;

        /* Atomic block read */
        if (ReadFile(context->inFile, buffer, blockSizeBytes, &bytesIO, NULL) == FALSE)
        {
            context->error = TRUE;
            ReleaseMutex(context->readMutex);
            break;
        }
        context->readOffset += bytesIO;
        ReleaseMutex(context->readMutex);

        /* During execution of this function, one of the remaining threads can read the consecutive data */
        convertBlock(buffer, context->wordSize, context->blockSize);

        /* Now we check if it is our turn to write */
        /* It is our turn when others threads have written up to the point where we have read */
        if(WaitForSingleObject(context->writeMutex, INFINITE) != WAIT_OBJECT_0)
            break;
        writeOffsetLocal = context->writeOffset;
        ReleaseMutex(context->writeMutex);
        while (writeOffsetLocal != readOffsetLocal && !context->error)
        {
            /* If not, yield execution and keep polling until it is */
            SwitchToThread();
            if(WaitForSingleObject(context->writeMutex, INFINITE) != WAIT_OBJECT_0)
                break;
            writeOffsetLocal = context->writeOffset;
            ReleaseMutex(context->writeMutex);
        }
        /* Atomic block write*/
        if (WaitForSingleObject(context->writeMutex, INFINITE) != WAIT_OBJECT_0)
            break;

        /* TODO write block here */
        context->writeOffset = writeOffsetLocal + blockSizeBytes;
        ReleaseMutex(context->writeMutex);  
    }
    HeapFree(GetProcessHeap(), 0, buffer);
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

        switch (word_size)
        {
        case 2:
            convertWord2(buffer);
            break;
        case 4:
            convertWord4(buffer);
            break;
        default:
            convertWord(buffer, word_size);
            break;
        }
        

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
            *progress = (int) (100.0 * ((float)totalWritten / size.QuadPart));
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