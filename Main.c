#include "Logging.h"
#include "ProcessManager.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "memwatch.h"

#define REFRESH_RATE 5

MonitorRequest** monitorRequests;
int configLength;
RegisterEntry* root;

bool sigintReceived = false;

void cleanupGlobals()
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    destructChain(root);
}

void sigkillChildHandler(int signum)
{
    if (signum == SIGKILL_CHILD)
    {
        cleanupGlobals();
        printf("Debug: %d dying...\n", getpid());
        exit(0);
    }
}

void sigintHandler(int signum)
{
    if (signum == SIGINT)
    {
        sigintReceived = true;
    }
}


int main(int argc, char** argv)
{
    signal(SIGINT, sigintHandler);
    signal(SIGKILL_CHILD, sigkillChildHandler);

    if (!killOtherProcNannys())
    {
        exit(-1);
    }
    
    LogReport parentInfo;
    parentInfo.message = stringNumberJoin("Parent process is PID ", getpid());
    parentInfo.type = INFO;
    saveLogReport(parentInfo);
    free(parentInfo.message);
    parentInfo.message = NULL;

    monitorRequests = NULL;
    configLength = getProcessesToMonitor(argc, argv, &monitorRequests);

    if (configLength == -1)
    {
        // Already logged
        return -1;
    }

    root = constuctorRegisterEntry((pid_t)0, NULL, NULL);
    RegisterEntry* tail = root;
    
    while (!sigintReceived)
    {
        refreshRegisterEntries(root);
        int i;
        for (i = 0; i < configLength; ++i)
        {
            setupMonitoring(monitorRequests[i]->processName, monitorRequests[i]->monitorDuration, root, tail);
            while(tail->next != NULL)
            {
                // Refresh tail
                tail = tail->next;
            }
        }
        sleep(REFRESH_RATE);
    }

    killAllChildren(root);
    cleanupGlobals();
    printf("Debug: %d dying...\n", getpid());
    return 0;
}