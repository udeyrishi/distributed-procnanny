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