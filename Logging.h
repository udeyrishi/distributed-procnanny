#ifndef __LOGGING__
#define __LOGGING__

typedef enum { FATAL, INFO, ACTION, WARNING, ERROR, DEBUG } LogType;

typedef struct
{
    LogType type;
    char* message;
} LogReport;

void saveLogReport(LogReport message);

#endif