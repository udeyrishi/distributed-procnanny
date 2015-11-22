#include "CommunicationManager.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "Utils.h"
#include "memwatch.h"

uint32_t readUInt(int sock)
{
    char bytes[4];
    assert(readData(sock, bytes, 4) == 4);
    return ntohl(*(uint32_t *)bytes);
}

void writeUInt(int sock, uint32_t num)
{
    char* bytes = (char *)htonl(num);
    assert(writeData(sock, bytes, sizeof(num)) == sizeof(num));
}

char* readString(int fd, LoggerPointer logger)
{
    uint32_t messageLength = readUInt(fd);
    char* string = (char*)malloc(sizeof(char)*messageLength);

    LogReport error;
    if (!checkMallocResult(string, &error))
    {
        logger(error, false);
        return (char*)-1;
    }

    int size = readData(fd, string, messageLength);
    if (size < 0)
    {
        error.message = stringNumberJoin("Reading data failed from fd: ", fd);
        error.type = ERROR;
        logger(error, false);
        free(error.message);
        return (char*)-1;
    }
    assert(size == messageLength);
    return string;
}

void writeString(int fd, char* string)
{
    size_t length = strlen(string);
    writeUInt(fd, length);
    writeData(fd, string, length);
}

// Source: https://eclass.srv.ualberta.ca/mod/resource/view.php?id=1766878
// Jim Frost tutorial
size_t writeData(int fd, const void* buffer, size_t size)
{
    size_t total = 0;

    while (total < size)
    {
        size_t thisTime;
        if ((thisTime = write(fd, buffer, size - total)) < 0)
        {
            return -1;
        }
        else
        {
            total += thisTime;
            buffer += thisTime;
        }
    }

    return total;
}

size_t readData(int fd, void* buffer, size_t size)
{ 
    size_t total = 0;

    while (total < size)
    {
        size_t thisTime;
        if ((thisTime = read(fd, buffer, size - total)) < 0)
        {
            return -1;
        }
        else
        {
            total += thisTime;
            buffer += thisTime;
        }
    }

    return total;
}

void manageReads(fd_set* activeFileDescriptors, 
                 struct timeval* timeout, 
                 bool* quit,
                 DataReceivedCallback dataReceivedCallback, 
                 LoggerPointer logger)
{
    while (!(*quit))
    {
        fd_set readFileDescriptors = *activeFileDescriptors;
        if (select(FD_SETSIZE, &readFileDescriptors, NULL, NULL, timeout) < 0)
        {
            // select return negative value is a signal was received
            if (quit)
            {
                return;
            }
            LogReport report;
            report.message = "Failed to wait on the read file descriptors using select";
            report.type = ERROR;
            logger(report, false);
            exit(-1);
        }

        int i;
        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &readFileDescriptors))
            {
                dataReceivedCallback(i);
            }
        }
    }
}