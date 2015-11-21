#include <stdio.h>
#include "Logging.h"
#include "Process.h"
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
	printf("Hello, world!\n");
	return 0;
}