#include <stdio.h>
#include "Logging.h"
#include "memwatch.h"

#define PORT 3010

int main(int argc, char** argv) 
{
	logServerInfo(PORT);
	printf("Hello, world!\n");
	return 0;
}