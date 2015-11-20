#include "MonitorRequest.h"
#include "Logging.h"
#include "Utils.h"
#include "ProgramIO.h"
#include "memwatch.h"

MonitorRequest* constructMonitorRequest(char* requestString)
{
    MonitorRequest* this = (MonitorRequest*)malloc(sizeof(MonitorRequest));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        saveLogReport(report);
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

int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests)
{
    LogReport report;
    report.message = (char*)NULL;

    if (argc < 2)
    {
        report.message = "Config file path needed as argument.";
        report.type = ERROR;
        saveLogReport(report);
        printLogReport(report);
        return -1;
    }

    char* configPath = argv[1];
    int configLines = 0;
    char** config = readFile(configPath, &configLines, &report);
    
    if (report.message != NULL)
    {
        if (configLines > 0)
        {
            freeOutputFromProgram(config, configLines);
        }
        saveLogReport(report);
        return -1;
    }

    // Array of MonitorRequest pointers
    MonitorRequest** requests = (MonitorRequest**)malloc(configLines*sizeof(MonitorRequest*));
    if (!checkMallocResult(requests, &report))
    {
        freeOutputFromProgram(config, configLines);
        saveLogReport(report);
        return -1;
    }

    int i;
    for (i = 0; i < configLines; ++i) 
    {
        MonitorRequest* request = constructMonitorRequest(config[i]);

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