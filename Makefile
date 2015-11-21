CC = gcc
CFLAGS = -Wall -DMEMWATCH -DMW_STDIO
SERVER_SRC = server/ServerMain.c server/Logging.c
SERVER_OBJS = $(SERVER_SRC:.c=.o)
SERVER_TARGET = procnanny.server

all:
	$(CC) -Iclient -Icommon -Iserver $(CFLAGS) server/Logging.c client/ClientMain.c server/ServerMain.c common/MonitorRequest.c client/Process.c client/ProcessManager.c common/ProgramIO.c client/RegisterEntry.c common/Utils.c common/memwatch.c -o procnanny

server: $(SERVER_OBJS)
	$(CC) -Iserver -Icommon $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC)

.c.o:
	$(CC) $(CFLAGS) -Icommon -c $<  -o $@

clean:
	$(RM) procnanny.client $(SERVER_TARGET) procnanny