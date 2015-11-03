#include "Logging.h"
#include "ProcessManager.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "memwatch.h"

#define REFRESH_RATE 5

MonitorRequest** monitorRequests = NULL;
int configLength = 0;
RegisterEntry* root = NULL;

bool sigintReceived = false;
bool readConfig = true;

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
        closeChildEndsOfPipes();
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

void sighupHandler(int signum)
{
    if (signum == SIGHUP)
    {
        readConfig = true;
    }
}

int main(int argc, char** argv)
{
    signal(SIGINT, sigintHandler);
    signal(SIGKILL_CHILD, sigkillChildHandler);
    signal(SIGHUP, sighupHandler);

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

    root = constuctorRegisterEntry((pid_t)0, NULL, NULL);
    RegisterEntry* tail = root;
    
    int killCount = 0;

    bool isRetry = false;
    while (!sigintReceived)
    {
        if (readConfig)
        {
            logSighupCatch(argv[1]);
            readConfig = false;
            destroyMonitorRequestArray(monitorRequests, configLength);
            monitorRequests = NULL;
            configLength = 0;
            configLength = getProcessesToMonitor(argc, argv, &monitorRequests);
            isRetry = false;

            if (configLength == -1)
            {
                // Already logged
                return -1;
            }
        }

        killCount += refreshRegisterEntries(root);
        int i;
        for (i = 0; i < configLength; ++i)
        {
            setupMonitoring(isRetry, monitorRequests[i]->processName, monitorRequests[i]->monitorDuration, root, tail);
            while(tail->next != NULL)
            {
                // Refresh tail
                tail = tail->next;
            }
        }
        sleep(REFRESH_RATE);
        isRetry = true;
    }

    killAllChildren(root);
    cleanupGlobals();
    logFinalReport(killCount);
    return 0;
}