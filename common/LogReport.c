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

#include "LogReport.h"
#include "Utils.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "memwatch.h"

// private
char* getTime()
{
    char* output;
    time_t rawtime;
    time(&rawtime);
    char* t = ctime(&rawtime);
    t[strlen(t) - 1] = '\0';
    output = stringJoin("[", t);
    char* output2 = stringJoin(output, "] ");
    free(output);
    return output2;
}

char* getFormattedReport(LogReport report)
{
    char* output;
    char* currentTime = getTime();
    switch(report.type)
    {
        case ERROR:
            output = stringJoin(currentTime, "Error: ");
            break;
        case WARNING:
            output = stringJoin(currentTime, "Warning: ");
            break;
        case ACTION:
            output = stringJoin(currentTime, "Action: ");
            break;
        case FATAL:
            output = stringJoin(currentTime, "Fatal: ");
            break;
        case INFO:
            output = stringJoin(currentTime, "Info: ");
            break;
        case DEBUG:
            output = stringJoin(currentTime, "Debug: ");
            break;
        case SERVER_INFO:
            output = stringJoin(currentTime, "procnanny server: ");
            break;
        default:
            output = stringJoin(currentTime, "Unknown: ");
            break;
    }
    free(currentTime);
    currentTime = NULL;
    char* output2 = stringJoin(output, report.message);
    free(output);
    return output2;
}

void printLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    printf("%s\n", output);
    free(output);
}