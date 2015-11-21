CC = gcc
CFLAGS = -Wall -DMEMWATCH -DMW_STDIO

INCLUDES = -Icommon

SERVER_SRC = server/ServerMain.c server/Logging.c common/Utils.c common/memwatch.c
SERVER_OBJS = $(SERVER_SRC:.c=.o)
SERVER_TARGET = procnanny.server

all:
	$(CC) -Iclient -Icommon -Iserver $(CFLAGS) server/Logging.c client/ClientMain.c common/MonitorRequest.c client/Process.c client/ProcessManager.c common/ProgramIO.c client/RegisterEntry.c common/Utils.c common/memwatch.c -o procnanny

server: $(SERVER_OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) procnanny.client $(SERVER_TARGET) procnanny *.o *~ server/*.o client/*.o common/*.o