#ifndef __PROC_READER__
#define __PROC_READER__

#include "Logging.h"

typedef struct 
{
	int pid;
	char* tty;
	char* time;
	char* cmd;
} Process;

char** getOutputFromProgram(const char* programName, int maxNumberLines, int maxLineLength, int * numberLinesRead, LogReport* report); 

Process* getRunningProcesses(int maxNumberOfProcesses, int maxProcessLength, int* processesFound, LogReport* report);

void createProcess(char* processString, Process* this);

void destroyProcess(Process* this);

#endif