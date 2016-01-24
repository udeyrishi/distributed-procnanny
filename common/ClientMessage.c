/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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