#ifndef __COMMUNICATION_MANAGER__
#define __COMMUNICATION_MANAGER__

#include <sys/select.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "LogReport.h"
#include "OperationResult.h"

// This should drain all the data from fd
typedef void (* DataReceivedCallback)(int fd);
typedef void (* TimeoutCallback)(void);
typedef void (* PausedCallback)(void);

OperationResult_uint32_t readUInt(int sock, LoggerPointer logger); // Done
OperationResult_uint32_t writeUInt(int sock, uint32_t num, LoggerPointer logger);

OperationResult_string readString(int fd, LoggerPointer logger);
OperationResult_string writeString(int fd, const char* string, LoggerPointer logger);

OperationResult_raw writeData(int fd, const void* buffer, size_t size, LoggerPointer logger);
OperationResult_raw readData(int fd, void* buffer, size_t size, LoggerPointer logger);

void manageReads(const fd_set* activeFileDescriptors,
                 const struct timeval* timeout,
                 const bool* quit,
                 const bool* pause,
                 const PausedCallback onPaused,
                 const DataReceivedCallback onDataReceived,
                 const TimeoutCallback onTimeout,
                 const LoggerPointer logger);

#endif