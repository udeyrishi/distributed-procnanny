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

#define CHILD (pid_t)0

const char* PROGRAM_NAME = "procnanny";

void killOtherProcNannys()
{
    int num = 0;
    Process** procs = searchRunningProcesses(&num, PROGRAM_NAME);
    if (procs == NULL)
    {
        if (num == 0)
        {
            // Nothing found
            return;
        }
        exit(-1);
    }

    int i;
    for(i = 0; i < num; ++i)
    {
        Process* p = procs[i];
        if (getpid() != p->pid)
        {
            LogReport report;
            report.message = stringNumberJoin("Another procnanny found. Killing it. PID: ", (int)p->pid);
            report.type = NORMAL;
            saveLogReport(report);
            safeFree(report.message);
            
            if(!killProcess(*p))
            {
                report.message = stringNumberJoin("Failed to kill another procnanny. PID: ", (int)p->pid);
                report.type = ERROR;
                saveLogReport(report);
                safeFree(report.message);
            }
        }
    }

    destroyProcessArray(procs, num);
}

pid_t monitor(char* processName, unsigned long int duration, ProcessStatusCode* statusCode)
{
    int num = 0;
    Process** procs = searchRunningProcesses(&num, processName);
    if (procs == NULL)
    {
        if (num == 0)
        {
            *statusCode = NOT_FOUND;
            return getpid();
        }
        exit(-1);
    }

    int i;
    
    pid_t pid = getpid();
    
    for(i = 0; i < num; ++i)
    {
        Process* p = procs[i];
        
        if (p -> pid == getpid())
        {
            // config contains procnanny...not happening...
            // TODO: log it
            continue;
        }

        pid = p -> pid;
        switch (pid = fork())
        {
            case CHILD:
                sleep(duration);
                int newNum = 0;
                Process** processRecheck = searchRunningProcesses(&newNum, processName);
                if (processRecheck == NULL)
                {
                    if (newNum == 0)
                    {
                        *statusCode = DIED;
                    }
                    else
                    {
                        *statusCode = FAILED;
                    }
                }
                else
                {
                    int checkCounter;
                    for (checkCounter = 0; checkCounter < newNum; ++checkCounter)
                    {
                        if (processRecheck[checkCounter] -> pid == p -> pid)
                        {
                            if(killProcess(*p))
                            {
                                *statusCode = KILLED;
                            }
                            else
                            {
                                *statusCode = FAILED;
                            }
                            break;
                        }
                    }

                    if (checkCounter == newNum)
                    {
                        *statusCode = DIED;
                    }
                }
                destroyProcessArray(processRecheck, newNum);
                i = num;
                break;

            case -1:
                destroyProcessArray(procs, num);
                exit(-1);

            default:
                // parent
                break;
        }
    }

    destroyProcessArray(procs, num);
    return pid;
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
    unsigned long int duration = strtoul(config[0], NULL, 10);

    int i;
    ProcessStatusCode status = (ProcessStatusCode)-99; // Invalid
    bool isChild = false;

    for (i = 1; i < configLength; ++i)
    {
        char* processName = config[i];

        if (monitor(processName, duration, &status) == CHILD)
        {
            isChild = true;
            break;
        }
        else if ((int)status != -99)
        {
            // TODO better NOT_FOUND and FAILED logging.
            printf("Parent returned status %d when trying to set up a child monitor process.\n", (int)status);
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
        while((pid = wait(&status)) != -1)
        {
            // TODO : proper logging
            if (status == 0)
            {
                printf("Killed proc. pid: %d.\n", (int)pid);
            }
            else if (status < 0)
            {
                printf("Failed to kill proc. pid: %d.\n", (int)pid);
            }
            else
            {
                printf("Process died before timing: %d.\n", (int)pid);
            }
        }
        return 0;
    }
}