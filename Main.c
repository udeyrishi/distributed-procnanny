#include "Logging.h"
#include "ProcessManager.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "memwatch.h"

#define REFRESH_RATE 5


int main(int argc, char** argv)
{
    if (!killOtherProcNannys())
    {
        exit(-1);
    }

    logParentInit();
    int killCount = monitor(REFRESH_RATE, argc, argv);
    logFinalReport(killCount);
    return 0;
}