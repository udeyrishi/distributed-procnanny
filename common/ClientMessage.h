#ifndef __CLIENT_MESSAGE__
#define __CLIENT_MESSAGE__

#include "LogReport.h"

#define LOG_MESSAGE (char)0xFFFF
#define INT_ACK (char)0xFFFE

char readClientMessageStatusCode(int sock, LoggerPointer logger);
LogReport readLogMessage(int sock, LoggerPointer logger);
#endif