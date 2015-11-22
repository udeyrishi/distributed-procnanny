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
int getProcessesToMonitor(const char* configPath, MonitorRequest*** monitorRequests, LoggerPointer saveLogReport);

bool sendConfig(int sock, int configLength, MonitorRequest** newConfig, LoggerPointer logger);
int readConfig(int sock, MonitorRequest*** requestBuffer, LoggerPointer logger);

#endif