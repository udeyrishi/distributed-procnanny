#include "Logging.h"
#include "ProcessManager.h"
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