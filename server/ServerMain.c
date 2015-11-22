#include <stdio.h>
#include "Logging.h"
#include "Process.h"
#include "MonitorRequest.h"
#include "Server.h"
#include "memwatch.h"

#define PORT 3010
const char* PROGRAM_NAME = "procnanny.server";

MonitorRequest** monitorRequests = NULL;
int configLength = 0;

void destroyGlobals()
{
    
    destroyMonitorRequestArray(monitorRequests, configLength);
}

void logger(LogReport report, bool verbose)
{
    saveLogReport(report);
    if (verbose)
    {
        printLogReport(report);
    }
}

void rereadConfig(int argc, char** argv)
{
    destroyMonitorRequestArray(monitorRequests, configLength);
    monitorRequests = NULL;
    configLength = 0;
    configLength = getProcessesToMonitor(argc, argv, &monitorRequests, logger);

    if (configLength == -1)
    {
        // Already logged
        exit(-1);
    }
}

int main(int argc, char** argv) 
{
    logServerInfo(PORT);
    
    if (!killOtherProcessAndVerify(PROGRAM_NAME, logger))
    {
        exit(-1);
    }

    rereadConfig(argc, argv);

    int mainSocket = makeServerSocket(PORT, logger);
    if (mainSocket < 0)
    {
        exit(-1);
    }

    destroyGlobals();
    close(mainSocket);
    return 0;
}