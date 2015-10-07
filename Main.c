#include "Logging.h"
#include "ProcessReader.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include "memwatch.h"

const char* PROGRAM_NAME = "procnanny";


void killOtherProcNannys()
{
    int num = 0;
    Process* procs = getRunningProcesses(&num);
    if (procs == NULL)
    {
        exit(-1);
    }

    int i;
    for(i = 0; i < num; ++i)
    {
        Process p = procs[i];
        if (strcmp(p.cmd, PROGRAM_NAME) == 0 && getpid() != p.pid)
        {
            LogReport report;
            report.message = stringNumberJoin("Another procnanny found. Killing it. PID: ", (int)p.pid);
            report.type = NORMAL;
            saveLogReport(report);
            safeFree(report.message);
            if(!killProcess(p))
            {
                report.message = stringNumberJoin("Failed to kill another procnanny. PID: ", (int)p.pid);
                report.type = ERROR;
                saveLogReport(report);
                safeFree(report.message);
            }
        }
    }

    destroyProcessArray(procs, num);
}


int main(int argc, char** argv)
{
    killOtherProcNannys();
    
    char** config = NULL;
    int configLength = getProcessesToMonitor(argc, argv, &config);
    if (configLength == -1)
    {
        return -1;
    }
    int duration = atoi(config[0]);
    printf("%d\n", duration);
	int numberProcesses;
    Process* processes = getRunningProcesses(&numberProcesses);
    if (processes == NULL)
    {
        return -1;
    }

    freeOutputFromProgram(config, configLength);
    destroyProcessArray(processes, numberProcesses);

    return 0;
}