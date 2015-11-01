#include "Logging.h"
#include "ProcessManager.h"
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
    
    LogReport parentInfo;
    parentInfo.message = stringNumberJoin("Parent process is PID ", getpid());
    parentInfo.type = INFO;
    saveLogReport(parentInfo);
    free(parentInfo.message);

    MonitorRequest** monitorRequests = NULL;
    int configLength = getProcessesToMonitor(argc, argv, &monitorRequests);

    if (configLength == -1)
    {
        // Already logged
        return -1;
    }

    int i;
    ProcessStatusCode status;
    bool isChild = false;

    RegisterEntry* root = constuctorRegisterEntry((pid_t)0, NULL, NULL);
    RegisterEntry* tail = root;

    for (i = 0; i < configLength; ++i)
    {
        char* processName = monitorRequests[i]->processName;
        unsigned long int duration = monitorRequests[i]->monitorDuration;

        status = (ProcessStatusCode)-99; // Invalid code for initial value
        
        if (monitor(processName, duration, &status, tail) == CHILD)
        {
            isChild = true;
            break;
        }
        else if ((int)status == -99)
        {
            // Normal parent execution
            while(tail->next != NULL)
            {
                // Refresh tail
                tail = tail->next;
            }
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

    destroyMonitorRequestArray(monitorRequests, configLength);

    if (isChild)
    {
        destructChain(root);
        return (int)status;
    }
    else
    {
        int status = 0;
        pid_t pid;
        int killCount = 0;
        while((pid = wait(&status)) != -1)
        {
            int monitorDuration = 0;
            Process* killedProcess = findMonitoredProcess(pid, root, &monitorDuration);
            if (status == 0)
            {
                ++killCount;
                logProcessKill(killedProcess->pid, killedProcess->command, monitorDuration);
            }
            else if (status < 0)
            {
                LogReport report;
                report.message = stringNumberJoin("Failed to kill process with PID: ", (int)killedProcess->pid);
                report.type = ERROR;
                saveLogReport(report);
                free(report.message);
            }
            else
            {
                logSelfDying(killedProcess->pid, killedProcess->command, monitorDuration);
            }
            free(killedProcess);
        }

        logFinalReport(killCount);
        destructChain(root);
        return 0;
    }
}