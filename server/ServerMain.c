#include <stdio.h>
#include "Logging.h"
#include "Process.h"
#include "MonitorRequest.h"
#include "Server.h"
#include "memwatch.h"

#define PORT 3010
const char* PROGRAM_NAME = "procnanny.server";

int main(int argc, char** argv)
{
    logServerInfo(PORT);

    if (!killOtherProcessAndVerify(PROGRAM_NAME, logger))
    {
        exit(-1);
    }

    if (argc < 2)
    {
        LogReport report;
        report.message = "Config file path needed as argument.";
        report.type = ERROR;
        logger(report, true);
        return -1;
    }

    return createServer(PORT, argv[1]);
}