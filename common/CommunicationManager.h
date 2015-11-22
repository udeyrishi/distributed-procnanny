#ifndef __COMMUNICATION_MANAGER__
#define __COMMUNICATION_MANAGER__

#include <sys/select.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "LogReport.h"

// This should drain all the data from fd
typedef void (* DataReceivedCallback)(int fd);

uint32_t readUInt(int sock);
void writeUInt(int sock, uint32_t num);

char* readString(int fd, LoggerPointer logger);
void writeString(int fd, char* string);

size_t writeData(int fd, const void* buffer, size_t size);
size_t readData(int fd, void* buffer, size_t size);

void manageReads(fd_set* activeFileDescriptors, 
                 struct timeval* timeout, 
                 bool* quit,
                 DataReceivedCallback dataReceivedCallback, 
                 LoggerPointer logger);

#endif