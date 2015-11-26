#ifndef __COMMUNICATION_MANAGER__
#define __COMMUNICATION_MANAGER__

#include "LogReport.h"
#include <sys/select.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// This should drain all the data from fd
typedef void (* DataReceivedCallback)(int fd);
typedef void (* TimeoutCallback)(void);
typedef void (* PausedCallback)(void);

uint32_t readUInt(int sock, LoggerPointer logger);
bool writeUInt(int sock, uint32_t num, LoggerPointer logger);

char* readString(int fd, LoggerPointer logger);
bool writeString(int fd, char* string, LoggerPointer logger);

ssize_t writeData(int fd, const void* buffer, size_t size, LoggerPointer logger);
ssize_t readData(int fd, void* buffer, size_t size, LoggerPointer logger);

void manageReads(const fd_set* activeFileDescriptors,
                 const struct timeval* timeout,
                 const bool* quit,
                 const bool* pause,
                 const PausedCallback onPaused,
                 const DataReceivedCallback onDataReceived,
                 const TimeoutCallback onTimeout,
                 const LoggerPointer logger);

#endif