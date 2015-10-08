#include <stdio.h>
#include "Logging.h"
#include "Utils.h"
#include <time.h>
#include <string.h>
#include "memwatch.h"

// private
char* getTime()
{
    char* output;
    time_t rawtime;
    time (&rawtime);
    char* t = ctime(&rawtime);
    t[strlen(t) - 1] = '\0';
    output = stringJoin("[", t);
    char* output2 = stringJoin(output, "] ");
    free(output);
    return output2;
}

// private
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

void saveLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    printf("%s\n", output);
    free(output);
}

void printLogReport(LogReport report)
{
    char* output = getFormattedReport(report);
    printf("%s\n", output);
    free(output);
}

// TODO: Maybe for next part, write an easier method for general purpose string sizes...
void logFinalReport(int killCount)
{
    LogReport report;
    report.type = INFO;
    char* message = stringNumberJoin("Exiting. ", killCount);
    char* message2 = stringJoin(message, " process(es) killed.");
    free(message);
    report.message = message2;
    saveLogReport(report);
    printLogReport(report);
    free(message2);
}

void logProcessMonitoringInit(char* processName, pid_t pid)
{
    LogReport report;
    char* message = stringJoin("Initializing monitoring of process '", processName); 
    char* message2 = stringJoin(message, "' (PID ");
    free(message);
    message = stringNumberJoin(message2, (int)pid);
    free(message2);
    message2 = stringJoin(message, ").");
    free(message);
    message = NULL;
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report);
    free(message2);
}

void logProcessKill(pid_t pid, const char* name, unsigned long int duration)
{
    LogReport report;
    char* message = stringNumberJoin("PID ", pid);
    char* message2 = stringJoin(message, " (");
    free(message);
    message = stringJoin(message2, name);
    free(message2);
    message2 = stringJoin(message, ") killed after exceeding ");
    free(message);
    message = stringULongJoin(message2, duration);
    free(message2);
    message2 = stringJoin(message, " seconds.");
    free(message);
    report.message = message2;
    report.type = ACTION;
    saveLogReport(report);
    free(report.message);
}

void logSelfDying(pid_t pid, const char* name, unsigned long int duration)
{
    LogReport report;
    char* message = stringNumberJoin("PID ", pid);
    char* message2 = stringJoin(message, " (");
    free(message);
    message = stringJoin(message2, name);
    free(message2);
    message2 = stringJoin(message, ") died on its own by ");
    free(message);
    message = stringULongJoin(message2, duration);
    free(message2);
    message2 = stringJoin(message, " seconds.");
    free(message);
    report.message = message2;
    report.type = INFO;
    saveLogReport(report);
    free(report.message);
}