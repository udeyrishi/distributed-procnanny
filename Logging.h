#ifndef __LOGGING__
#define __LOGGING__

#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum { FATAL, INFO, ACTION, WARNING, ERROR, DEBUG } LogType;

typedef struct
{
    LogType type;
    char* message;
} LogReport;

bool appendToFile(const char* path, char* string);
void saveLogReport(LogReport message);
void printLogReport(LogReport report);
void logFinalReport(int killCount);
void logProcessMonitoringInit(char* processName, pid_t pid);
void logProcessKill(pid_t pid, const char* name, unsigned long int duration);
void logSelfDying(pid_t pid, const char* name, unsigned long int duration);
#endif