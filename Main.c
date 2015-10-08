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