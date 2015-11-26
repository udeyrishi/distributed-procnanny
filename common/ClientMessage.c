#include "ClientMessage.h"
#include "CommunicationManager.h"
#include "Utils.h"
#include <assert.h>
#include "memwatch.h"

ClientMessageStatusCode readClientMessageStatusCode(int sock, LoggerPointer logger)
{
    ClientMessageStatusCode messageCode;
    ssize_t size = readData(sock, &messageCode, sizeof(messageCode), logger);
    if (size < 0)
    {
        exit(-1);
    }
    assert(size == sizeof(ClientMessageStatusCode));
    return messageCode;
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