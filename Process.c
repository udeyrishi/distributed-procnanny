#include "Process.h"
#include "Logging.h"
#include "Utils.h"
#include <string.h>
#include "memwatch.h"

// Constructor of a Process struct from a processString (output line from the ps command)
Process* processConstructor(char* processString)
{
    Process* this = (Process*)malloc(sizeof(Process));
    LogReport report;
    if (!checkMallocResult(this, &report))
    {
        saveLogReport(report);
        return (Process*)NULL;
    }

    this->user = getNextStrTokString(processString);
    this->pid = atoi(strtok(NULL, " "));
    this->cpu = atof(strtok(NULL, " "));
    this->mem = atof(strtok(NULL, " "));
    this->vsz = atoi(strtok(NULL, " "));
    this->rss = atoi(strtok(NULL, " "));
    this->tty = getNextStrTokString(NULL);
    this->stat = getNextStrTokString(NULL);
    this->start = getNextStrTokString(NULL);
    this->time = getNextStrTokString(NULL);

    this->command = (char*)malloc(sizeof(char));
    *this->command = '\0';

    char* commandPart;
    while ((commandPart = strtok(NULL, " ")) != NULL)
    {
        char* spaceAdded;
        if (*this->command == '\0')
        {
            spaceAdded = this->command;
        }
        else
        {
            spaceAdded = stringJoin(this->command, " ");
            free(this->command);
        }

        char* newJoin = stringJoin(spaceAdded, commandPart);
        free(spaceAdded);

        this->command = newJoin;
    }
    return this;
}

// Destructor for a process
void processDestructor(Process* this)
{
    if (this == NULL)
    {
        return;
    }
    free(this->user);
    free(this->tty);
    free(this->stat);
    free(this->start);
    free(this->time);
    free(this->command);
    free(this);
}

void destroyProcessArray(Process** array, int count)
{
    if (array == NULL) 
    {
        return;
    }

    int i;
    for (i = 0; i < count; ++i)
    {
        processDestructor(array[i]);
    }
    free(array);
}
