#include "MonitorRequest.h"
#include "Utils.h"
#include "ProgramIO.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "memwatch.h"

MonitorRequest* monitorRequestConstructor(char* processName, uint32_t monitorDuration, LoggerPointer logger)
{
    MonitorRequest* this = (MonitorRequest*)malloc(sizeof(MonitorRequest));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        logger(report, false);
        return NULL;
    }

    this->processName = copyString(processName);
    this->monitorDuration = monitorDuration;

    return this;
}

MonitorRequest* constructMonitorRequest(char* requestString, LoggerPointer saveLogReport)
{
    char* processName = getNextStrTokString(requestString);
    char* monitorDuration = getNextStrTokString(NULL);
    unsigned long int duration = strtoul(monitorDuration, NULL, 10);
    free(monitorDuration);
    monitorDuration = NULL;
    assert(duration <= (unsigned long int)UINT32_MAX);
    MonitorRequest* this = monitorRequestConstructor(processName, (uint32_t)duration, saveLogReport);
    free(processName);
    return this;
}

void destroyMonitorRequest(MonitorRequest* this)
{
    free(this->processName);
    free(this);
}

void destroyMonitorRequestArray(MonitorRequest** requestArray, int size)
{
    if (requestArray == NULL)
    {
        return;
    }

    int i;
    for (i = 0; i < size; ++i)
    {
        destroyMonitorRequest(requestArray[i]);
    }

    free(requestArray);
}

int getProcessesToMonitor(const char* configPath, MonitorRequest*** monitorRequests, LoggerPointer saveLogReport)
{
    LogReport report;
    report.message = (char*)NULL;

    int configLines = 0;
    char** config = readFile(configPath, &configLines, &report);

    if (report.message != NULL)
    {
        if (configLines > 0)
        {
            freeOutputFromProgram(config, configLines);
        }
        saveLogReport(report, false);
        return -1;
    }

    // Array of MonitorRequest pointers
    MonitorRequest** requests = (MonitorRequest**)malloc(configLines*sizeof(MonitorRequest*));
    if (!checkMallocResult(requests, &report))
    {
        freeOutputFromProgram(config, configLines);
        saveLogReport(report, false);
        return -1;
    }

    int i;
    for (i = 0; i < configLines; ++i)
    {
        MonitorRequest* request = constructMonitorRequest(config[i], saveLogReport);

        if (request == NULL)
        {
            freeOutputFromProgram(config, configLines);
            destroyMonitorRequestArray(requests, configLines);
            return -1;
        }
        requests[i] = request;
    }

    freeOutputFromProgram(config, configLines);
    *monitorRequests = requests;
    return configLines;
}

bool sendConfig(int sock, int configLength, MonitorRequest** newConfig, LoggerPointer logger)
{
    assert(configLength >= 0);
    assert(newConfig != NULL);

    if (!writeUInt(sock, (uint32_t)configLength, logger))
    {
        LogReport report;
        report.message =  stringNumberJoin("Failed to send config file to client at socket: ", sock);
        report.type = ERROR;
        logger(report, false);
        free(report.message);
        return false;
    }
    int i;
    for (i = 0; i < configLength; ++i)
    {
        if (!(writeString(sock, newConfig[i]->processName, logger) &&
              writeUInt(sock, newConfig[i]->monitorDuration, logger)))
        {
            LogReport report;
            report.message =  stringNumberJoin("Failed to send config file to client at socket: ", sock);
            report.type = ERROR;
            logger(report, false);
            free(report.message);
            return false;
        }
    }

    return true;
}

int readConfig(int sock, MonitorRequest*** requestBuffer, LoggerPointer logger)
{
    OperationResult_uint32_t configReadStatus = readUInt(sock, logger);
    if (configReadStatus.rawStatus.status != DONE)
    {
        LogReport report;
        report.type = ERROR;
        report.message = "Failed to read config from server. Error code: ";

        char* statusString = operationStatusToString(configReadStatus.rawStatus.status);
        report.message = stringJoin(report.message, statusString);
        logger(report, false);
        free(report.message);
        return -1;
    }
    if (configReadStatus.data > (uint32_t)MAX_INT)
    {
        LogReport report;
        report.message = "Config length greater than max int value";
        report.type = ERROR;
        logger(report, false);
        return -1;
    }
    int configLength = (int)configReadStatus.data;
    MonitorRequest** requests = (MonitorRequest**)malloc(sizeof(MonitorRequest*)*configLength);

    LogReport error;
    if (!checkMallocResult(requests, &error))
    {
        logger(error, false);
        return -1;
    }

    int i;
    char* processName;
    uint32_t monitorDuration;
    for (i = 0; i < configLength; ++i)
    {
        OperationResult_string procNameStatus = readString(sock, logger);
        if (procNameStatus.rawStatus.status != DONE)
        {
            destroyMonitorRequestArray(requests, i);
            return -1;
        }
        processName = procNameStatus.data;

        OperationResult_uint32_t monitorDurationStatus = readUInt(sock, logger);
        if (monitorDurationStatus.rawStatus.status != DONE)
        {
            destroyMonitorRequestArray(requests, i);
            return -1;
        }


        monitorDuration = monitorDurationStatus.data;
        requests[i] = monitorRequestConstructor(processName, monitorDuration, logger);
        free(processName);
    }

    *requestBuffer = requests;
    return configLength;
}
