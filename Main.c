#include "Logging.h"
#include "ProcessReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "memwatch.h"

#define MAX_PROCESSES 128
#define MAX_PROCESS_LINE_LENGTH 200


int main(int argc, char** argv)
{
    LogReport report;
    report.message = (char*)NULL;

    if (argc < 2)
    {
        report.message = "Config file path needed as argument";
        report.type = FATAL;
        saveLogReport(report);
        return -1;
    }

    char* configPath = argv[1];
    int configLines = 0;
    char** config = readFile(configPath, MAX_PROCESSES + 1, MAX_PROCESS_LINE_LENGTH, &configLines, &report);
    
    if (report.message != NULL)
    {
        if (configLines > 0)
        {
            freeOutputFromProgram(config, configLines);
        }
        saveLogReport(report);
        return -1;
    }

    if (configLines < 2)
    {
        freeOutputFromProgram(config, configLines);
        report.message = "Bad config file. Number of lines should be greater than 1.";
        report.type = FATAL;
        saveLogReport(report);
        return -1;
    }

    int duration = atoi(config[0]);
    printf("%d\n", duration);
    int i;
    for (i = 1; i < configLines; ++i)
    {
        printf("%s\n", config[i]);
    }

    freeOutputFromProgram(config, configLines);

/*
	int i;
    LogReport report;
    report.message = NULL;
    Process* processes = getRunningProcesses(MAX_PROCESSES, MAX_PROCESS_LINE_LENGTH, &i, &report);

    if (report.message != NULL)
    {
        saveLogReport(report);
        return -1;
    }

    int j;
    for(j = 0; j < i; ++j)
    {
        Process this = processes[j];
        printf("PID: %d, TTY: %s, TIME: %s, CMD: %s\n", this.pid, this.tty, this.time, this.cmd);
        destroyProcess(&this);
    }

    free(processes);
*/
    

    return 0;
}