#ifndef __LOGGING__
#define __LOGGING__

typedef enum { FATAL, ERROR, WARNING, NORMAL } LogType;

typedef struct
{
    LogType type;
    char* message;
} LogReport;

void saveLogReport(LogReport message);

#endif