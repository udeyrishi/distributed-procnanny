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

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "LogReport.h"
#include <sys/types.h>
#include <stdbool.h>

typedef struct
{
	char* user;
	pid_t pid;
	double cpu;
	double mem;
	int vsz;
	int rss;
	char* tty;
	char* stat;
	char* start;
	char* time;
	char* command;
} Process;

void destroyProcessArray(Process** array, int count);
Process* processConstructor(char* processString, LoggerPointer saveLogReport);
void processDestructor(Process* this);
Process** searchRunningProcesses(int* processesFound, const char* processName, bool ignoreCmdOptions, LoggerPointer saveLogReport);
bool killProcess(Process process);
bool killOtherProcessAndVerify(const char* programName, LoggerPointer saveLogReport);
#endif