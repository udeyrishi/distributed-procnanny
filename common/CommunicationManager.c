#include "CommunicationManager.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <limits.h>
#include "Utils.h"
#include "memwatch.h"

OperationResult_uint32_t readUInt(int sock, LoggerPointer logger)
{
    char bytes[4];

    OperationResult_uint32_t result;
    result.rawStatus = readData(sock, bytes, sizeof(bytes), logger);
    result.data = ntohl(*(uint32_t *)bytes);

    return result;
}

OperationResult_uint32_t writeUInt(int sock, uint32_t num, LoggerPointer logger)
{
    num = htonl(num);
    char* bytes = (char *)&num;

    OperationResult_uint32_t result;
    result.rawStatus = writeData(sock, bytes, sizeof(num), logger);
    result.data = num;

    return result;
}

OperationResult_string readString(int fd, LoggerPointer logger)
{
    OperationResult_uint32_t messageLength = readUInt(fd, logger);

    OperationResult_string result;
    result.data = NULL;

    if (messageLength.rawStatus.status != DONE)
    {
        result.rawStatus = messageLength.rawStatus;
        return result;
    }

    char* string = (char*)malloc(sizeof(char)*messageLength.data);

    LogReport error;
    if (!checkMallocResult(string, &error))
    {
        logger(error, false);
        exit(-1);
    }

    result.rawStatus = readData(fd, string, messageLength, logger);

    if (result.rawStatus.status == DONE)
    {
        result.data = string;
    }
    else
    {
        free(string);
    }

    result.rawStatus.dataSize += messageLength.rawStatus.dataSize;
    return result;
}

OperationResult_string writeString(int fd, const char* string, LoggerPointer logger)
{
    size_t length = strlen(string) + 1;

    OperationResult_string result;
    result.data = NULL;

    OperationResult_uint32_t lengthWriteStatus = writeUInt(fd, length, logger);
    if (lengthWriteStatus.rawStatus.status != DONE)
    {
        result.rawStatus = lengthWriteStatus.rawStatus;
        return result;
    }

    result.rawStatus = writeData(fd, string, length, logger);
    if (result.rawStatus == DONE)
    {
        result.data = string;
    }

    result.rawStatus.dataSize += lengthWriteStatus.rawStatus.dataSize;

    return result;
}

// Source: https://eclass.srv.ualberta.ca/mod/resource/view.php?id=1766878
// Jim Frost tutorial
OperationResult_raw writeData(int fd, const void* buffer, size_t size, LoggerPointer logger)
{
    if (size > (size_t)SSIZE_MAX)
    {
        LogReport report;
        report.message = "size can't be bigger than SSIZE_MAX";
        report.type = DEBUG;
        logger(report, false);
        exit(-1);
    }

    OperationResult_raw result;
    result.status = DONE;
    result.dataSize = 0;

    while (result.dataSize < size)
    {
        ssize_t thisTime = write(fd, buffer, size - result.dataSize);
        if (thisTime < 0)
        {
            LogReport report;
            report.message = stringNumberJoin("Write failed to fd: ", fd);
            report.type = ERROR;
            logger(report, false);
            free(report.message);

            result.status = FAILED;
            break;
        }
        else if (thisTime == 0)
        {
            result.status = FD_CLOSED;
            break;
        }
        else
        {
            result.dataSize += (size_t)thisTime;
            buffer += (size_t)thisTime;
        }
    }

    return result;
}

OperationResult_raw readData(int fd, void* buffer, size_t size, LoggerPointer logger)
{
    if (size > (size_t)SSIZE_MAX)
    {
        LogReport report;
        report.message = "size can't be bigger than SSIZE_MAX";
        report.type = DEBUG;
        logger(report, false);
        exit(-1);
    }

    OperationResult_raw result;
    result.status = DONE;
    result.dataSize = 0;

    while (result.dataSize < size)
    {
        ssize_t thisTime = read(fd, buffer, size - result.dataSize);
        if (thisTime < 0)
        {
            LogReport error;
            error.message = stringNumberJoin("Reading data failed from fd: ", fd);
            error.type = ERROR;
            logger(error, false);
            free(error.message);

            result.status = FAILED;
            break;
        }
        else if (thisTime == 0)
        {
            result.status = FD_CLOSED;
            break;
        }
        else
        {
            result.dataSize += (size_t)thisTime;
            buffer += (size_t)thisTime;
        }
    }

    return result;
}

// private
struct timeval* initTimeout(const struct timeval* timeout)
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
    return timeoutCopy;
}

void manageReads(const fd_set* activeFileDescriptors,
                 const struct timeval* timeout,
                 const bool* quit,
                 const bool* pause,
                 const PausedCallback onPaused,
                 const DataReceivedCallback onDataReceived,
                 const TimeoutCallback onTimeout,
                 const LoggerPointer logger)
{
    struct timeval* timeoutCopy = initTimeout(timeout);

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
            if (*quit)
            {
                break;
            }
            if (*pause)
            {
                if (onPaused != NULL)
                {
                    onPaused();
                }
            }
            else
            {
                LogReport report;
                report.message = "Failed to wait on the read file descriptors using select";
                report.type = ERROR;
                logger(report, false);
                exit(-1);
            }
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