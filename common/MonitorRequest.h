/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef __MONITOR_REQUEST_H__
#define __MONITOR_REQUEST_H__

#include "LogReport.h"
#include <stdint.h>

typedef struct
{
	char* processName;
	uint32_t monitorDuration;
} MonitorRequest;

MonitorRequest* monitorRequestConstructor(char* processName, uint32_t monitorDuration, LoggerPointer logger);
MonitorRequest* constructMonitorRequest(char* requestString, LoggerPointer saveLogReport);
void destroyMonitorRequest(MonitorRequest* this);
void destroyMonitorRequestArray(MonitorRequest** requestArray, int size);
int getProcessesToMonitor(const char* configPath, MonitorRequest*** monitorRequests, LoggerPointer saveLogReport);

bool sendConfig(int sock, int configLength, MonitorRequest** newConfig, LoggerPointer logger);
int readConfig(int sock, MonitorRequest*** requestBuffer, LoggerPointer logger);

#endif