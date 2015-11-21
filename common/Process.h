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