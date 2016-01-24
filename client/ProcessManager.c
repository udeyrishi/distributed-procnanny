/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ProcessManager.h"
#include "Logging.h"
#include "Utils.h"
#include "MonitorRequest.h"
#include "RegisterEntry.h"
#include "ProgramIO.h"
#include "ServerMessage.h"
#include "CommunicationManager.h"
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include "memwatch.h"

static int killCount  = 0;
static RegisterEntry* tail = NULL;

static int writingToParent = 0;
static int readingFromParent = 0;

static MonitorRequest** monitorRequests = NULL;
static int configLength = 0;

static bool sigintReceived = false;

static fd_set activeFds;
static int serverSocket = -1;

//private
void cleanupGlobals()
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    destructChain();
}

//private
ProcessStatusCode childMain(pid_t pid, int duration)
{
    sleep(duration);
    int result = kill(pid, SIGKILL);
    if (result == 0)
    {
        return KILLED;
    }
    else
    {
        return DIED;
    }
}

//private
void setupMonitoring(bool isRetry, char* processName, unsigned long int duration, RegisterEntry* tail)
{
    int num = 0;
    Process** runningProcesses = searchRunningProcesses(&num, processName, false, saveLogReport);
    if (runningProcesses == NULL)
    {
        // Nothing to be done
        if (num == 0)
        {
            if (!isRetry)
            {
                LogReport report;
                report.message = stringJoin("No process found with name: ", processName);
                report.type = INFO;
                saveLogReport(report, false);
                free(report.message);
            }

            return;
        }
        exit(-1);
    }

    int i;
    for(i = 0; i < num; ++i)
    {
        Process* p = runningProcesses[i];

        if (p -> pid == getpid())
        {
            // If procnannys were killed in the beginning, but a new one was started in between and the user expects to track that.
            // Should never happen/be done.
            LogReport report;
            report.message = "Config file had procnanny as one of the entries. It will be ignored if no other procnanny is found.";
            report.type = WARNING;
            saveLogReport(report, false);
            continue;
        }

        if (isProcessAlreadyBeingMonitored(p->pid))
        {
            continue;
        }

        logProcessMonitoringInit(processName, p->pid);

        RegisterEntry* freeChild = getFirstFreeChild();
        if (freeChild == NULL)
        {
            // fork a new child
            int writeToChildFD[2];
            int readFromChildFD[2];
            if (pipe(writeToChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report, true);
                // TODO: Kill all children
                // TODO: Log all final kill count
                exit(-1);
            }

            if (pipe(readFromChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report, true);
                // TODO: Kill all children
                // TODO: Log all final kill count
                exit(-1);
            }

            pid_t forkPid = fork();
            switch (forkPid)
            {
                case -1:
                    destroyProcessArray(runningProcesses, num);
                    exit(-1);

                case CHILD:
                    close(writeToChildFD[1]);
                    close(readFromChildFD[0]);
                    writingToParent = readFromChildFD[1];
                    readingFromParent = writeToChildFD[0];
                    pid_t targetPid = p->pid;
                    destroyProcessArray(runningProcesses, num);
                    cleanupGlobals();

                    while (true)
                    {
                        ProcessStatusCode childStatus = childMain(targetPid, duration);
                        write(writingToParent, &childStatus, 1);

                        MonitorMessage message;
                        assert(read(readingFromParent, &message, sizeof(MonitorMessage)) == sizeof(MonitorMessage));
                        targetPid = message.targetPid;
                        duration = message.monitorDuration;
                    }

                    break;

                default:
                     // parent
                    close(writeToChildFD[0]);
                    close(readFromChildFD[1]);
                    tail->monitoringProcess = forkPid;
                    tail->monitoredProcess = p->pid;
                    tail->monitorDuration = duration;
                    tail->monitoredName = copyString(p->command);
                    tail->startingTime = time(NULL);
                    tail->isAvailable = false;
                    tail->writeToChildFD = writeToChildFD[1];
                    tail->readFromChildFD = readFromChildFD[0];
                    FD_SET(tail->readFromChildFD, &activeFds);
                    tail->next = constuctorRegisterEntry((pid_t)0, NULL, NULL);
                    tail = tail->next;
                    break;
            }

        }
        else
        {
            // use freeChild
            freeChild->isAvailable = false;
            freeChild->monitoredProcess = p->pid;
            free(freeChild->monitoredName);
            freeChild->monitoredName = copyString(p->command);
            freeChild->monitorDuration = duration;
            freeChild->startingTime = time(NULL);
            MonitorMessage message;
            message.targetPid = p->pid;
            message.monitorDuration = duration;
            write(freeChild->writeToChildFD, &message, sizeof(MonitorMessage));
        }
    }

    destroyProcessArray(runningProcesses, num);
}

void refreshMonitoring(bool isRetry)
{
    int i;
    for (i = 0; i < configLength; ++i)
    {
        setupMonitoring(isRetry, monitorRequests[i]->processName, monitorRequests[i]->monitorDuration, tail);
        while(tail->next != NULL)
        {
            // Refresh tail
            tail = tail->next;
        }
    }
}

void readMessageFromServer()
{
    ServerMessage message = readMessage(serverSocket, saveLogReport);

    switch (message.type)
    {
        case HUP:
            destroyMonitorRequestArray(monitorRequests, configLength);
            configLength = message.configLength;
            monitorRequests = message.newConfig;
            refreshMonitoring(false);
            break;

        case INT:
            sigintReceived = true;
            break;

        default:
            // error already logged
            exit(-1);
    }
}

void dataReceivedCallback(int fd)
{
    if (fd == serverSocket)
    {
        readMessageFromServer();
    }
    else
    {
        if (didChildKill(fd))
        {
            ++killCount;
        }
    }
}

void timeoutCallback()
{
    refreshMonitoring(true);
}

int monitor(int refreshRate, int serverSock)
{
    serverSocket = serverSock;
    FD_ZERO(&activeFds);
    FD_SET(serverSocket, &activeFds);

    configLength = readConfig(serverSocket, &monitorRequests, saveLogReport);

    tail = initialiseRegister();

    refreshMonitoring(false);

    struct timeval timeout;
    timeout.tv_sec = refreshRate;
    timeout.tv_usec = 0;

    bool pause = false;
    manageReads(&activeFds, &timeout, &sigintReceived, &pause, NULL, dataReceivedCallback, timeoutCallback, saveLogReport);

    killCount += refreshRegisterEntries();

    killAllChildren();
    cleanupGlobals();
    return killCount;
}