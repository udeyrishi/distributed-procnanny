#include "MonitorRequest.h"
#include "Logging.h"
#include "Utils.h"
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
