#ifndef __MONITOR_REQUEST_H__
#define __MONITOR_REQUEST_H__

typedef struct
{
	char* processName;
	unsigned long int monitorDuration;
} MonitorRequest;

MonitorRequest* constructMonitorRequest(char* requestString);
void destroyMonitorRequest(MonitorRequest* this);
void destroyMonitorRequestArray(MonitorRequest** requestArray, int size);
int getProcessesToMonitor(int argc, char** argv, MonitorRequest*** monitorRequests);

#endif