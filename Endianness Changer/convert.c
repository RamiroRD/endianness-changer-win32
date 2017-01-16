#include "stdafx.h"
#include <math.h>

#include "convert.h"

#define DEFAULT_THREAD_AMOUNT     4
#define DEFAULT_BLOCK_SIZE      512

typedef struct
{
    HANDLE  inFile;
    HANDLE  outFile;
    volatile DWORD64 readOffset;  /* next byte to read */
    volatile DWORD64 writeOffset; /* next byte to write */
    HANDLE  readMutex;   /* protects readOffset and provides read exclusion */
    HANDLE  writeMutex;  /* protects writeOffset and provides write exclusion */
    DWORD64 totalWords;
    PWORD   progress;
    WORD    blockSize;   /* amount of words to process at once */
    volatile WORD    error;
    volatile PBOOL   cancel;
    UCHAR   wordSize;
    
} CONVERSIONSTRUCT;



static void convertWord(BYTE * word_ptr, UCHAR word_size)
{
    int i;
    BYTE aux;
    for (i = 0; i < word_size / 2; i++)
    {
        aux = word_ptr[word_size - i - 1];
        word_ptr[word_size - i - 1] = word_ptr[i];
        word_ptr[i] = aux;
    }
}

static void convertWord2(BYTE * word_ptr)
{
    char aux = word_ptr[0];
    word_ptr[0] = word_ptr[1];
    word_ptr[1] = aux;
}

static void convertWord4(BYTE * word_ptr)
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

static void convertBlock(BYTE * word_ptr, const UCHAR word_size, WORD n)
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

/* This function is to be run by each thread, hence its ugly signature */
static DWORD WINAPI processBlock(LPVOID args)
{
    CONVERSIONSTRUCT * context = (CONVERSIONSTRUCT*)args;
    DWORD64 readOffsetLocal;
    DWORD64 writeOffsetLocal;
    DWORD blockSizeBytes = context->wordSize * context->blockSize;
    
    /* Should always be the same as the block size in bytes after every IO operation */
    DWORD   bytesIO;

    /* Block buffer */
    BYTE * buffer = HeapAlloc(GetProcessHeap(),
                    0,
                    blockSizeBytes);
    if (!buffer)
    {
        context->error = TRUE;
        return 0xFFFFFFFF;
    }
    
    /* This loop will be executed as long as there are bytes left to read and there is no error */
    while(!context->error && !*(context->cancel))
    {
        /* We find out the offset of the next block to read, process and write */
        if (WaitForSingleObject(context->readMutex, INFINITE) != WAIT_OBJECT_0)
        {
            context->error = TRUE;
            ReleaseMutex(context->readMutex);
            return FALSE;
        }
        readOffsetLocal = context->readOffset;

        /* Have all the words already been read? */
        if (readOffsetLocal == context->wordSize * context->totalWords)
        {
            /* We are done, release the lock and exit */
            ReleaseMutex(context->readMutex);
            return TRUE;
        }
        
        /* For last block read, reduce block size */
        if (context->totalWords * context->wordSize - readOffsetLocal < blockSizeBytes)
            blockSizeBytes =(DWORD) ( context->totalWords * context->wordSize - readOffsetLocal );

        /* Atomic block read */
        if (ReadFile(context->inFile, buffer, blockSizeBytes, &bytesIO, NULL) == FALSE)
        {
            context->error = TRUE;
            ReleaseMutex(context->readMutex);
            return FALSE;
        }
        context->readOffset += bytesIO;
        ReleaseMutex(context->readMutex);

        /* During execution of this function, one of the remaining threads can read the consecutive data */
        convertBlock(buffer, context->wordSize, context->blockSize);

        /* Now we check whether it is our turn to write */
        /* It is our turn when others threads have written up to the point where we have read */
        if(WaitForSingleObject(context->writeMutex, INFINITE) != WAIT_OBJECT_0)
            return FALSE;
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
        /* Atomic block write */
        if (WaitForSingleObject(context->writeMutex, INFINITE) != WAIT_OBJECT_0)
            return FALSE;

        if (WriteFile(context->outFile, buffer, blockSizeBytes, &bytesIO, NULL) == FALSE)
        {
            context->error = TRUE;
            ReleaseMutex(context->writeMutex);
            return FALSE;
        }
        context->writeOffset = writeOffsetLocal + bytesIO;

        *(context->progress) = context->writeOffset * 100 / (context->totalWords * context->wordSize);

        ReleaseMutex(context->writeMutex);  
    }
    HeapFree(GetProcessHeap(), 0, buffer);
    return TRUE;
}

BOOL isPowerOfTwo(UCHAR x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

BOOL checkArguments(CONVERTARGS * args)
{
    if (!args)
        return FALSE;
    // Check if word_size is a power of 2.
    if (!isPowerOfTwo(args->wordSize))
        return FALSE;

    /* Fail if source missing */
    if (args->src == NULL)
        return FALSE;

    /* Fail if destination missing */
    if (args->des == NULL)
        return FALSE;

    return TRUE;
}

BOOL convertFiles(CONVERTARGS * args)
{
    HANDLE * threadsArray;

    if (!checkArguments(args))
        return FALSE;

    /* We do these checks here because we need the size later on */
    LARGE_INTEGER size;
    if (!GetFileSizeEx(args->src, &size))
        return FALSE;

    /* Fail if size not multiple of word size*/
    if (size.QuadPart % args->wordSize)
        return FALSE;

    if (args->threads == 0)
        args->threads = DEFAULT_THREAD_AMOUNT;

    /* Fill out context structure to be shared by all threads */
    CONVERSIONSTRUCT context;
    context.inFile = args->src;
    context.outFile = args->des;
    context.readOffset = 0;
    context.writeOffset = 0;
    context.readMutex = CreateMutex(NULL, FALSE, NULL);
    context.writeMutex = CreateMutex(NULL, FALSE, NULL);
    context.blockSize = (args->blockSize == 0) ? DEFAULT_BLOCK_SIZE : args->blockSize;
    context.wordSize = args->wordSize;
    context.cancel = &args->cancel;
    context.progress = &args->progress;
    context.totalWords = size.QuadPart / args->wordSize;
    context.error = 0;

    /* Avoid spawning threads that will never process anything */
    if ((size.QuadPart / args->wordSize) / context.blockSize < args->threads)
        args->threads = 1 + (UCHAR)((size.QuadPart / args->wordSize) / context.blockSize);

    /* Allocate array of threads */
    threadsArray = HeapAlloc(GetProcessHeap(),
        HEAP_GENERATE_EXCEPTIONS,
        args->threads * sizeof(HANDLE));

    /* Spawn threads */
    for (UCHAR i = 0; i < args->threads; i++)
    {
        threadsArray[i] = CreateThread(NULL,
            0,
            processBlock,
            (LPVOID) &context,
            0,
            NULL);
    }

    /* Wait for termination of all threads */
    WaitForMultipleObjects(args->threads,
        threadsArray,
        TRUE,
        INFINITE);

    CloseHandle(context.readMutex);
    CloseHandle(context.writeMutex);

    HeapFree(GetProcessHeap(), 0, threadsArray);
    return TRUE; /* TODO: return TRUE only if all went well */
}