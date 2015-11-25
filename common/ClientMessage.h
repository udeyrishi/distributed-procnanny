#ifndef __CLIENT_MESSAGE__
#define __CLIENT_MESSAGE__

#include "LogReport.h"

typedef char ClientMessageStatusCode;

#define LOG_MESSAGE (ClientMessageStatusCode)0xFFFF
#define INT_ACK (ClientMessageStatusCode)0xFFFE

ClientMessageStatusCode readClientMessageStatusCode(int sock, LoggerPointer logger);
bool writeClientMessageStatusCode(int sock, ClientMessageStatusCode statusCode, LoggerPointer logger);
LogReport readLogMessage(int sock, const char* clientName, LoggerPointer logger);
bool writeLogMessage(int serverSocket, LogReport report, LoggerPointer logger);
#endif