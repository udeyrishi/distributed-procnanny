#include "ClientMessage.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "Utils.h"
#include "memwatch.h"

char readClientMessageStatusCode(int sock, LoggerPointer logger)
{
    char messageCode;
    ssize_t oneByte = readData(sock, &messageCode, sizeof(char), logger);
    if (oneByte < 0)
    {
        exit(-1);
    }
    assert(oneByte == sizeof(char));
    return messageCode;
}

LogReport readLogMessage(int sock, const char* clientName,  LoggerPointer logger)
{
    LogReport report;
    report.message = readString(sock, logger);
    if (report.message < 0)
    {
        exit(-1);
    }
    ssize_t size = readData(sock, (char *)&(report.type), sizeof(report.type), logger);
    if (size < 0)
    {
        exit(-1);
    }
    assert(size == sizeof(report.type));
    char* c = stringJoin(report.message, " on node ");
    free(report.message);
    report.message = stringJoin(c, clientName);
    free(c);
    return report;
}