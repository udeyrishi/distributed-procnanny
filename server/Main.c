#include "Logging.h"
#include "ProcessManager.h"
#include "memwatch.h"

#define REFRESH_RATE 5

int main(int argc, char** argv)
{
    logParentInit();
	
    if (!killOtherProcNannys())
    {
        exit(-1);
    }

    int killCount = monitor(REFRESH_RATE, argc, argv);
    logFinalReport(killCount);
    return 0;
}