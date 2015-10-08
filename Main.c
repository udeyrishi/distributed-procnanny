#include "Logging.h"
#include "ProcessReader.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "memwatch.h"

int main(int argc, char** argv)
{
    if (!killOtherProcNannys())
    {
        exit(-1);
    }
    
    char** config = NULL;
    int configLength = getProcessesToMonitor(argc, argv, &config);
    if (configLength == -1)
    {
        // Already logged
        return -1;
    }
    unsigned long int duration = strtoul(config[0], NULL, 10);

    int i;
    ProcessStatusCode status;
    bool isChild = false;

    for (i = 1; i < configLength; ++i)
    {
        char* processName = config[i];
        status = (ProcessStatusCode)-99; // Invalid code for initial value
        
        if (monitor(processName, duration, &status) == CHILD)
        {
            isChild = true;
            break;
        }
        else if ((int)status == -99)
        {
            continue;
        }
        else if (status == NOT_FOUND)
        {
            LogReport report;
            report.message = stringJoin("No process found with name: ", processName);
            report.type = INFO;
            saveLogReport(report);
            free(report.message);
        }
        else
        {
            LogReport report;
            report.message = stringNumberJoin("Parent process returned unexpected status code while setting up a child process: ", (int)status);
            report.type = DEBUG;
            saveLogReport(report);
            free(report.message);
        }
    }

    freeOutputFromProgram(config, configLength);
    
    if (isChild)
    {
        return (int)status;
    }
    else
    {
        // FIX: this pid is the child's pid
        int status = 0;
        pid_t pid;
        int killCount = 0;
        while((pid = wait(&status)) != -1)
        {
            // TODO : proper logging
            if (status == 0)
            {
                ++killCount;
                LogReport report;
                report.message = stringNumberJoin("PID killed: ", pid);
                report.type = ACTION;
                saveLogReport(report);
                free(report.message);
                // TODO: Format:
                //Action: PID 332 (a.out) killed after exceeding 120 seconds.
            }
            else if (status < 0)
            {
                LogReport report;
                report.message = stringNumberJoin("Failed to kill process with PID: ", (int)pid);
                report.type = ERROR;
                saveLogReport(report);
                free(report.message);
            }
            else
            {
                LogReport report;
                // TODO: better format
                report.message = stringNumberJoin("Monitored process died automatically. PID: ", (int)pid);
                report.type = INFO;
                saveLogReport(report);
                free(report.message);
            }
        }

        logFinalReport(killCount);
        return 0;
    }
}