#include "CommunicationManager.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "Utils.h"
#include "memwatch.h"

uint32_t readUInt(int sock, LoggerPointer logger)
{
    char bytes[4];
    assert(readData(sock, bytes, 4, logger) == 4);
    return ntohl(*(uint32_t *)bytes);
}

bool writeUInt(int sock, uint32_t num, LoggerPointer logger)
{
    char* bytes = (char *)htonl(num);
    if (writeData(sock, bytes, sizeof(num), logger) < 0)
    {
        return false;
    }
    return true;
}

char* readString(int fd, LoggerPointer logger)
{
    uint32_t messageLength = readUInt(fd, logger);
    char* string = (char*)malloc(sizeof(char)*messageLength);

    LogReport error;
    if (!checkMallocResult(string, &error))
    {
        logger(error, false);
        return (char*)-1;
    }

    int size = readData(fd, string, messageLength, logger);
    if (size < 0)
    {
        free(string);
        return (char*)-1;
    }
    assert(size == messageLength);
    return string;
}

bool writeString(int fd, char* string, LoggerPointer logger)
{
    size_t length = strlen(string);

    if (!writeUInt(fd, length, logger) || writeData(fd, string, length, logger) < 0)
    {
        return false;
    }
    return true;
}

// Source: https://eclass.srv.ualberta.ca/mod/resource/view.php?id=1766878
// Jim Frost tutorial
size_t writeData(int fd, const void* buffer, size_t size, LoggerPointer logger)
{
    size_t total = 0;

    while (total < size)
    {
        size_t thisTime;
        if ((thisTime = write(fd, buffer, size - total)) < 0)
        {
            LogReport report;
            report.message = stringNumberJoin("Write failed to fd: ", fd);
            report.type = ERROR;
            logger(report, false);
            free(report.message);
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

size_t readData(int fd, void* buffer, size_t size, LoggerPointer logger)
{ 
    size_t total = 0;

    while (total < size)
    {
        size_t thisTime;
        if ((thisTime = read(fd, buffer, size - total)) < 0)
        {
            LogReport error;
            error.message = stringNumberJoin("Reading data failed from fd: ", fd);
            error.type = ERROR;
            logger(error, false);
            free(error.message);
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

void manageReads(const fd_set* activeFileDescriptors, 
                 const struct timeval* timeout, 
                 const bool* quit,
                 const DataReceivedCallback onDataReceived, 
                 const TimeoutCallback onTimeout,
                 const LoggerPointer logger)
{
    struct timeval* timeoutCopy;
    if (timeout == NULL)
    {
        timeoutCopy = NULL;
    }
    else
    {
        timeoutCopy = (struct timeval*)malloc(sizeof(struct timeval));
    }

    while (!(*quit))
    {
        // Need copying after every select call because select may modify the timeout value
        if (timeout != NULL)
        {
            memcpy(timeoutCopy, timeout, sizeof(struct timeval));
        }
        // Need copying because the caller may change in the activeFileDescriptors in the dataReceivedCallback
        // That change will only appear in the next iteration
        fd_set readFileDescriptors = *activeFileDescriptors;

        int numReady = select(FD_SETSIZE, &readFileDescriptors, NULL, NULL, timeoutCopy);
        if (numReady < 0)
        {
            // select return negative value is a signal was received -- no error in this case
            if (quit)
            {
                break;
            }
            LogReport report;
            report.message = "Failed to wait on the read file descriptors using select";
            report.type = ERROR;
            logger(report, false);
            exit(-1);
        }
        else if (numReady == 0)
        {
            if (onTimeout != NULL)
            {
                onTimeout();
            }
        }
        else
        {
            if (onDataReceived != NULL)
            {
                int i;
                for (i = 0; i < FD_SETSIZE; ++i)
                {
                    if (FD_ISSET(i, &readFileDescriptors))
                    {
                        onDataReceived(i);
                    }
                }
            }
        }
    }

    free(timeoutCopy);
}