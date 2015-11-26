#include "ClientMessage.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "Utils.h"
#include "memwatch.h"

ClientMessageStatusCode readClientMessageStatusCode(int sock, LoggerPointer logger)
{
    ClientMessageStatusCode messageCode;
    OperationResult_raw readStatus = readData(sock, &messageCode, sizeof(messageCode), logger);
    switch (readStatus.rawStatus.status)
    {
        case FAILED:
            return FAILED;

        case FD_CLOSED:
            return CLOSED;

        default:
            return messageCode;
    }
}

bool writeClientMessageStatusCode(int sock, ClientMessageStatusCode statusCode, LoggerPointer logger)
{
    ssize_t size = writeData(sock, &statusCode, sizeof(statusCode), logger);
    if (size < 0)
    {
        return false;
    }
    return true;
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

bool writeLogMessage(int serverSocket, LogReport report, LoggerPointer logger)
{
    if (!writeString(serverSocket, report.message, logger))
    {
        return false;
    }

    if (writeData(serverSocket, &(report.type), sizeof(report.type), logger) < 0)
    {
        return false;
    }

    return true;
}