#include "OperationResult.h"
#include "memwatch.h"

char* operationStatusToString(OperationStatus status)
{
    switch (status)
    {
        case DONE:
            return "DONE";
        case FAILED:
            return "FAILED";
        case FD_CLOSED:
            return "FD_CLOSED";
        default:
            return "UNKNOWN";
    }
}