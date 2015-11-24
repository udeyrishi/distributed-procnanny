//#include "Logging.h"
#include "LogReport.h"
#include <stdlib.h>
#include <stdio.h>
#include "Process.h"
#include "ProcessManager.h"
#include "Client.h"
#include "memwatch.h"

#define REFRESH_RATE 5
const char* PROGRAM_NAME = "procnanny.client";

int main(int argc, char** argv)
{
    // TODO: logParentInit();
    if (argc < 3)
    {
        LogReport report;
        report.message = "Server host name and port number needed as arguments.";
        report.type = ERROR;
        logger(report, true);
        return -1;
    }

    char* serverHostName = argv[1];
    int serverPort = atoi(argv[2]);
    int sock = makeClientSocket(serverHostName, serverPort);
    printf("socket: %d\n", sock);

    if (!killOtherProcessAndVerify(PROGRAM_NAME, logger))
    {
        exit(-1);
    }

    int killCount = monitor(REFRESH_RATE, argc, argv);
    printf("kill count: %d\n", killCount);
    //logFinalReport(killCount);
    return 0;
}