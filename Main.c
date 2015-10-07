#include "Logging.h"
#include "ProcessReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "memwatch.h"

int main(int argc, char** argv)
{
    char** config = NULL;
    int configLength = getProcessesToMonitor(argc, argv, &config);
    if (configLength == -1)
    {
        return -1;
    }
    int duration = atoi(config[0]);
    printf("%d\n", duration);
	int numberProcesses;
    Process* processes = getRunningProcesses(&numberProcesses);
    if (processes == NULL)
    {
        return -1;
    }

    freeOutputFromProgram(config, configLength);
    destroyProcessArray(processes, numberProcesses);

    return 0;
}