#include "ProcessManager.h"
//#include "Logging.h"
#include "Client.h"
#include "Utils.h"
#include "MonitorRequest.h"
#include "RegisterEntry.h"
#include "ProgramIO.h"
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include "memwatch.h"

static RegisterEntry* root = NULL;

static int writingToParent = 0;
static int readingFromParent = 0;

static MonitorRequest** monitorRequests = NULL;
static int configLength = 0;

static bool sigintReceived = false;
static bool sighupReceived = false;

//private
void cleanupGlobals()
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    destructChain(root);
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
void setupMonitoring(bool isRetry, char* processName, unsigned long int duration, RegisterEntry* head, RegisterEntry* tail)
{
    int num = 0;
    Process** runningProcesses = searchRunningProcesses(&num, processName, false, logger);
    if (runningProcesses == NULL)
    {
        // Nothing to be done
        if (num == 0)
        {
            if (!isRetry)
            {
                /*
                LogReport report;
                report.message = stringJoin("No process found with name: ", processName);
                report.type = INFO;
                saveLogReport(report);
                free(report.message);
                */
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
            /*
            LogReport report;
            report.message = "Config file had procnanny as one of the entries. It will be ignored if no other procnanny is found.";
            report.type = WARNING;
            saveLogReport(report);
            */
            continue;
        }

        if (isProcessAlreadyBeingMonitored(p->pid, head))
        {
            continue;
        }

        //logProcessMonitoringInit(processName, p->pid);

        RegisterEntry* freeChild = getFirstFreeChild(head);
        if (freeChild == NULL)
        {
            // fork a new child
            int writeToChildFD[2];
            int readFromChildFD[2];
            if (pipe(writeToChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                /*
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report);
                printLogReport(report);
                */
                // TODO: Kill all children
                // TODO: Log all final kill count
                exit(-1);
            }

            if (pipe(readFromChildFD) < 0)
            {
                destroyProcessArray(runningProcesses, num);
                /*
                LogReport report;
                report.message = "Pipe creation error when trying to monitor new process.";
                report.type = ERROR;
                saveLogReport(report);
                printLogReport(report);
                */
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

//int monitor(int refreshRate, int argc, char** argv)
int monitor(int refreshRate, int serverSocket)
{
    //signal(SIGINT, sigintHandler);
    //signal(SIGHUP, sighupHandler);
    configLength = readConfig(serverSocket, &monitorRequests, logger);

    root = constuctorRegisterEntry((pid_t)0, NULL, NULL);
    RegisterEntry* tail = root;
    
    int killCount = 0;
    //rereadConfig(argc, argv);
    bool isRetry = false;

    while (!sigintReceived)
    {
        if (sighupReceived)
        {
            //logSighupCatch(argv[1]);
            sighupReceived = false;
            isRetry = false;
            //rereadConfig(argc, argv);
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
        sleep(refreshRate);
        isRetry = true;
    }

    // Final refresh before exiting
    killCount += refreshRegisterEntries(root);

    killAllChildren(root);
    cleanupGlobals();
    return killCount;
}