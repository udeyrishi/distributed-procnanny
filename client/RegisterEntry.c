#include "RegisterEntry.h"
#include "Utils.h"
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "memwatch.h"

RegisterEntry* constuctorRegisterEntry(pid_t monitoringProcess, Process* monitoredProcess, RegisterEntry* next)
{
    RegisterEntry* entry = (RegisterEntry*)malloc(sizeof(RegisterEntry));
    entry->monitoringProcess = monitoringProcess;
    if (monitoredProcess == NULL)
    {
        entry->monitoredProcess = (pid_t)0;
        entry->monitoredName = NULL;
    }
    else
    {
        entry->monitoredProcess = monitoredProcess->pid;
        entry->monitoredName = copyString(monitoredProcess->command);
    }
    entry->next = next;
    return entry;
}

RegisterEntry* destructorRegisterEntry(RegisterEntry* this)
{
    if (this == NULL)
    {
        return NULL;
    }
    RegisterEntry* next = this->next;
    free(this->monitoredName);
    free(this);
    return next;
}

void destructChain(RegisterEntry* root)
{
    while (root != NULL)
    {
        root = destructorRegisterEntry(root);
    }
}

Process* findMonitoredProcess(pid_t monitoringProcess, RegisterEntry* reg, unsigned long int* duration)
{
    while (reg != NULL)
    {
        if (reg->monitoringProcess == monitoringProcess)
        {
            Process* found = (Process*)malloc(sizeof(Process));
            found->command = reg->monitoredName;
            found->pid = reg->monitoredProcess;
            *duration = reg->monitorDuration;
            return found;
        }
        else
        {
            reg = reg->next;
        }
    }
    return NULL;
}

bool isHeadNull(RegisterEntry* head)
{
    return (head == NULL || head->monitoringProcess == (pid_t)0);
}

int refreshRegisterEntries(RegisterEntry* head)
{
    int killed = 0;

    if (isHeadNull(head))
    {
        return killed;
    }

    time_t currentTime = time(NULL);
    while (head != NULL)
    {
        if (!(head->isAvailable) && (currentTime > head->startingTime + head->monitorDuration))
        {
            ProcessStatusCode message;
            assert(read(head->readFromChildFD, &message, 1) == 1);

            LogReport report;
            switch(message)
            {
                case DIED:
                    logSelfDying(head->monitoredProcess, head->monitoredName, head->monitorDuration);
                    break;

                case KILLED:
                    ++killed;
                    logProcessKill(head->monitoredProcess, head->monitoredName, head->monitorDuration);
                    break;

                case FAILED:
                    report.message = stringNumberJoin("Failed to kill process with PID: ", (int)head->monitoredProcess);
                    report.type = ERROR;
                    saveLogReport(report);
                    free(report.message);
                    break;

                default:
                    // TODO: UNEXPECTED
                    break;
            }

            head->isAvailable = true;
        }
        head = head->next;
    }

    return killed;
}

bool isProcessAlreadyBeingMonitored(pid_t pid, RegisterEntry* reg)
{
    while (!isHeadNull(reg))
    {
        if (reg->monitoredProcess == pid)
        {
            return true;
        }
        else
        {
            reg = reg->next;
        }
    }
    return false;
}

RegisterEntry* getFirstFreeChild(RegisterEntry* head)
{
    while (!isHeadNull(head))
    {
        if (head->isAvailable)
        {
            return head;
        }
        else 
        {
            head = head->next;
        }
    }

    return NULL;
}

void killAllChildren(RegisterEntry* root)
{
    while (!isHeadNull(root))
    {
        close(root->writeToChildFD);
        close(root->readFromChildFD);
        kill(root->monitoringProcess, SIGKILL_CHILD);
        root = root->next;
    }
}