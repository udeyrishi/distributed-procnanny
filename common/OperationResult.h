#ifndef __OPERATION_RESULT__
#define __OPERATION_RESULT__

#include <stdlib.h>
#include <stdbool.h>

typedef enum { DONE, FAILED, FD_CLOSED } OperationStatus;

// For raw read/write operations. External buffers needed for reads and writes
typedef struct
{
    OperationStatus status;

    // size of object read/written
    ssize_t dataSize;
} OperationResult_raw;

// For uint32_t reads/writes. For writes, confirmation of what was written (including any
// endianness changes that might have been done). For
// read, the uint32_t that was read.
typedef struct
{
    OperationResult_raw rawStatus;
    uint32_t data;
} OperationResult_uint32_t;

// For string read/writes. For writes, confirmation of what was written. For
// read, the string that was read. The pointer for write operation will echo the
// pointer to the buffer that was passed in. For read, the pointer will be dynamically
// malloced, so should be freed.
typedef struct
{
    OperationResult_raw rawStatus;
    char* data;
} OperationResult_string;

char* operationStatusToString(OperationStatus status);
#endif