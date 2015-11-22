#include <stdio.h>
#include "Logging.h"
#include "Process.h"
#include "Server.h"
#include "memwatch.h"

#define PORT 3010
const char* PROGRAM_NAME = "procnanny.server";

int main(int argc, char** argv) 
{
    logServerInfo(PORT);
    
    if (!killOtherProcessAndVerify(PROGRAM_NAME, saveLogReport))
    {
        exit(-1);
    }

    int mainSocket = makeServerSocket(PORT, saveLogReport);
    if (mainSocket < 0)
    {
        exit(-1);
    }

    close(mainSocket);
    return 0;
}