#ifndef __MONITOR_REQUEST_H__
#define __MONITOR_REQUEST_H__

#include "LogReport.h"

typedef struct
{
	char* processName;
	unsigned long int monitorDuration;
} MonitorRequest;

MonitorRequest* constructMonitorRequest(char* requestString, LoggerPointer saveLogReport);
void destroyMonitorRequest(MonitorRequest* this);
void destroyMonitorRequestArray(MonitorRequest** requestArray, int size);
int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests, LoggerPointer saveLogReport);

#endif