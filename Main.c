#include "Logging.h"
#include "ProcessReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "memwatch.h"

#define MAX_PROCESSES 128
#define MAX_PROCESS_LINE_LENGTH 200

int main(void)
{
	int i;
    LogReport report;
    Process* processes = getRunningProcesses(MAX_PROCESSES, MAX_PROCESS_LINE_LENGTH, &i, &report);
    printf("%d\n", i);

    int j;
    for(j = 0; j < i; ++j)
    {
        Process this = processes[j];
        printf("PID: %d, TTY: %s, TIME: %s, CMD: %s\n", this.pid, this.tty, this.time, this.cmd);
        destroyProcess(&this);
    }

    return 0;
}