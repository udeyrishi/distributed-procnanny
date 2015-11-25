#include "RegisterEntry.h"
#include "Utils.h"
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "Logging.h"
#include "memwatch.h"

typedef bool (* RegisterEntryPredicate)(const RegisterEntry* regEntry);
static int tempFD = -1;
static RegisterEntry* head = NULL;

RegisterEntry* initialiseRegister()
{
    head = constuctorRegisterEntry((pid_t)0, NULL, NULL);
    return head;
}

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

void destructChain()
{
    while (head != NULL)
    {
        head = destructorRegisterEntry(head);
    }
}

bool isNull(RegisterEntry* this)
{
    return (this == NULL || this->monitoringProcess == (pid_t)0);
}

RegisterEntry* findFirstChildSatisfying(RegisterEntryPredicate predicate)
{
    RegisterEntry* this = head;
    while (!isNull(this))
    {
        if (predicate(this))
        {
            return this;
        }
        else
        {
            this = this->next;
        }
    }

    return NULL;
}

bool isReadFDSameAsTempFD(const RegisterEntry* regEntry)
{
    return regEntry->readFromChildFD == tempFD;
}

bool didChildKill(int readFromChildFD)
{
    ProcessStatusCode message;
    assert(read(readFromChildFD, &message, sizeof(message)) == sizeof(message));

    tempFD = readFromChildFD;
    RegisterEntry* child = findFirstChildSatisfying(isReadFDSameAsTempFD);
    tempFD = -1;

    if (child == NULL)
    {
        LogReport report;
        report.message = "Message received from child not in the register.";
        report.type = DEBUG;
        saveLogReport(report, false);
        exit(-1);
    }

    LogReport report;
    bool result;
    switch(message)
    {
        case DIED:
            logSelfDying(child->monitoredProcess, child->monitoredName, child->monitorDuration);
            result = false;
            break;

        case KILLED:
            logProcessKill(child->monitoredProcess, child->monitoredName, child->monitorDuration);
            result = true;
            break;

        case FAILED:
            report.message = stringNumberJoin("Failed to kill process with PID: ", (int)child->monitoredProcess);
            report.type = INFO;
            saveLogReport(report, false);
            free(report.message);
            result = false;
            break;

        default:
            // TODO: UNEXPECTED
            result = false;
            break;
    }

    child->isAvailable = true;
    return result;
}

int refreshRegisterEntries()
{
    int killed = 0;
    time_t currentTime = time(NULL);

    RegisterEntry* this = head;
    while (!isNull(this))
    {
        if (!(this->isAvailable) && (currentTime > this->startingTime + this->monitorDuration))
        {
            if (didChildKill(this->readFromChildFD))
            {
                ++killed;
            }
        }
        this = this->next;
    }

    return killed;
}


bool isProcessAlreadyBeingMonitored(pid_t pid)
{
    RegisterEntry* reg = head;
    while (!isNull(reg))
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

bool isAvailable(const RegisterEntry* this)
{
    return this->isAvailable;
}

RegisterEntry* getFirstFreeChild()
{
    return findFirstChildSatisfying(isAvailable);
}

void killAllChildren()
{
    RegisterEntry* root = head;
    while (!isNull(root))
    {
        close(root->writeToChildFD);
        close(root->readFromChildFD);
        kill(root->monitoringProcess, SIGKILL_CHILD);
        root = root->next;
    }
}