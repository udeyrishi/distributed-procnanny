#include "Logging.h"
#include "ClientMessage.h"
#include "memwatch.h"

static int __serverSocket = -1;

void initializeLogger(int serverSocket)
{
	__serverSocket = serverSocket;
}

void saveLogReport(LogReport message, bool local)
{
	if (local)
    {
        printLogReport(message);
    }
    else
    {
    	if (!writeClientMessageStatusCode(__serverSocket, LOG_MESSAGE, saveLogReport) ||
    	    !writeLogMessage(__serverSocket, message, saveLogReport))
    	{
    		exit(-1);
    	}
    }
}

void logFinalReport(int killCount);
void logProcessMonitoringInit(char* processName, pid_t pid);
void logProcessKill(pid_t pid, const char* name, unsigned long int duration);
void logSelfDying(pid_t pid, const char* name, unsigned long int duration);