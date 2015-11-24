#include "ClientMessage.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "memwatch.h"

char readClientMessageStatusCode(int sock, LoggerPointer logger)
{
    char messageCode;
    size_t oneByte = readData(sock, &messageCode, sizeof(char), logger);
    if (oneByte < 0)
    {
        exit(-1);
    }
    assert(oneByte == sizeof(char));
    return messageCode;
}