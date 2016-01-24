/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "Logging.h"
#include "Process.h"
#include "ProcessManager.h"
#include "ClientSocket.h"
#include "CommunicationManager.h"
#include <stdlib.h>
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
        saveLogReport(report, true);
        return -1;
    }

    if (!killOtherProcessAndVerify(PROGRAM_NAME, saveLogReport))
    {
        exit(-1);
    }

    char* serverHostName = argv[1];
    int serverPort = atoi(argv[2]);
    int sock = makeClientSocket(serverHostName, serverPort);
    if (sock < 0)
    {
        return -1;
    }
    initializeLogger(sock);

    int killCount = monitor(REFRESH_RATE, sock);
    logFinalReport(killCount);
    close(sock);
    return 0;
}