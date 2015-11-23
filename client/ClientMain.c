//#include "Logging.h"
#include "LogReport.h"
#include <stdio.h>
#include "Process.h"
#include "ProcessManager.h"
#include "memwatch.h"

#define REFRESH_RATE 5
const char* PROGRAM_NAME = "procnanny";

int main(int argc, char** argv)
{
    //logParentInit();
	
    if (!killOtherProcessAndVerify(PROGRAM_NAME, logger))
    {
        exit(-1);
    }

    int killCount = monitor(REFRESH_RATE, argc, argv);
    printf("kill count: %d\n", killCount);
    //logFinalReport(killCount);
    return 0;
}