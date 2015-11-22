#include "MonitorRequest.h"
#include "Utils.h"
#include "ProgramIO.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "memwatch.h"

MonitorRequest* constructMonitorRequest(char* requestString, LoggerPointer saveLogReport)
{
    MonitorRequest* this = (MonitorRequest*)malloc(sizeof(MonitorRequest));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        saveLogReport(report, false);
        return NULL;
    }

    this->processName = getNextStrTokString(requestString);
    char* monitorDuration = getNextStrTokString(NULL);
    this->monitorDuration = strtoul(monitorDuration, NULL, 10);
    free(monitorDuration);
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
    int configLength = (int)readUInt(sock, logger);

    *requestBuffer = (MonitorRequest**)malloc(sizeof(MonitorRequest*)*configLength);

    LogReport error;
    if (!checkMallocResult(*requestBuffer, &error))
    {
        logger(error, false);
        return -1;
    }

    int i;
    for (i = 0; i < configLength; ++i)
    {
        char* processName = readString(sock, logger);
        unsigned long int monitorDuration = (unsigned long int)readUInt(sock, logger);
        char* line = stringULongJoin(processName, monitorDuration);
        free(processName);
        processName = NULL;
        (*requestBuffer)[i] = constructMonitorRequest(line, logger);
        free(line);
    }

    return configLength;
}
