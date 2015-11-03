#ifndef __PROCESS_H__
#define __PROCESS_H__

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

void destroyProcessArray(Process** array, int count);
Process* processConstructor(char* processString);
void processDestructor(Process* this);

#endif