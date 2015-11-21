#include "Logging.h"
#include "Process.h"
#include "ProcessManager.h"
#include "memwatch.h"

#define REFRESH_RATE 5
const char* PROGRAM_NAME = "procnanny";

int main(int argc, char** argv)
{
    logParentInit();
	
    if (!killOtherProcessAndVerify(PROGRAM_NAME))
    {
        exit(-1);
    }

    int killCount = monitor(REFRESH_RATE, argc, argv);
    logFinalReport(killCount);
    return 0;
}