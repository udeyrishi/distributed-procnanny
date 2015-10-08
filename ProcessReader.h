#ifndef __PROC_READER__
#define __PROC_READER__

#include "Logging.h"
#include "Utils.h"
#include <sys/types.h>

typedef int ProcessStatusCode;

#define DIED (ProcessStatusCode)2
#define NOT_FOUND (ProcessStatusCode)1
#define KILLED (ProcessStatusCode)0
#define FAILED (ProcessStatusCode)-1
#define CHILD (pid_t)0

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

char** getOutputFromProgram(const char* programName, int * numberLinesRead, LogReport* report); 
void freeOutputFromProgram(char** output, int numberLinesRead); 
Process** searchRunningProcesses(int* processesFound, const char* processName);
void destroyProcessArray(Process** array, int count);
void processConstructor(char* processString, Process* this);
void processDestructor(Process* this);
char** readFile(const char* filePath, int* numberLinesRead, LogReport* report);
int getProcessesToMonitor(int argc, char** argv, char*** configOutput);
bool killProcess(Process process);
void killOtherProcNannys();
pid_t monitor(char* processName, unsigned long int duration, ProcessStatusCode* statusCode);
#endif