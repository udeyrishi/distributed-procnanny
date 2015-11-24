CC = gcc
CFLAGS = -Wall -Werror -DMEMWATCH -DMW_STDIO

INCLUDES = -Icommon

SERVER_SRC = server/ServerMain.c \
			 server/Logging.c \
			 server/Server.c \
			 server/Client.c \
			 common/Utils.c \
			 common/Process.c \
			 common/ProgramIO.c \
			 common/LogReport.c \
			 common/MonitorRequest.c \
			 common/CommunicationManager.c \
			 common/ServerMessage.c \
			 common/ClientMessage.c \
			 common/memwatch.c
SERVER_OBJS = $(SERVER_SRC:.c=.o)
SERVER_TARGET = procnanny.server

CLIENT_SRC = client/ClientMain.c \
			 client/ProcessManager.c \
			 client/RegisterEntry.c \
			 client/Client.c \
			 common/Process.c \
			 common/Utils.c \
			 common/LogReport.c \
			 common/MonitorRequest.c \
			 common/ProgramIO.c \
			 common/CommunicationManager.c \
			 common/memwatch.c
CLIENT_OBJS = $(CLIENT_SRC:.c=.o)
CLIENT_TARGET = procnanny.client

BINS = $(CLIENT_TARGET) $(SERVER_TARGET)
OBJECT_FILES = *.o *~ server/*.o client/*.o common/*.o
LOGS = $(PROCNANNYLOGS) $(PROCNANNYSERVERINFO) memwatch.log

default: all

all: server client

client: $(CLIENT_OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS) 

server: $(SERVER_OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $<  -o $@

clean-log:
	$(RM) $(BINS) $(OBJECT_FILES) $(LOGS)

clean:
	$(RM) $(BINS) $(OBJECT_FILES)