#ifndef __PROC_READER__
#define __PROC_READER__

#include "Logging.h"
#include "Utils.h"
#include <sys/types.h>

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
//Process* getRunningProcesses(int* processesFound);
Process** searchRunningProcesses(int* processesFound, const char* processName);
void destroyProcessArray(Process** array, int count);
void processConstructor(char* processString, Process* this);
void processDestructor(Process* this);
char** readFile(const char* filePath, int* numberLinesRead, LogReport* report);
int getProcessesToMonitor(int argc, char** argv, char*** configOutput);
Boolean killProcess(Process process);
#endif