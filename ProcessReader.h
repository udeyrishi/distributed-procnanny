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

char** getOutputFromProgram(const char* programName, int * numberLinesRead, LogReport* report); 
void freeOutputFromProgram(char** output, int numberLinesRead); 
Process* getRunningProcesses(int* processesFound);
void destroyProcessArray(Process* array, int count);
void processConstructor(char* processString, Process* this);
void processDestructor(Process* this);
char** readFile(const char* filePath, int* numberLinesRead, LogReport* report);
int getProcessesToMonitor(int argc, char** argv, char*** configOutput);

#endif