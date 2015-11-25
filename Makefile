CC = gcc
CFLAGS = -Wall -Werror -DMEMWATCH -DMW_STDIO

INCLUDES = -Icommon

SERVER_SRC = server/ServerMain.c \
			 server/Logging.c \
			 server/Server.c \
			 server/ClientManager.c \
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
			 client/Logging.c \
			 common/Process.c \
			 common/Utils.c \
			 common/LogReport.c \
			 common/MonitorRequest.c \
			 common/ProgramIO.c \
			 common/ClientMessage.c \
			 common/CommunicationManager.c \
			 common/memwatch.c
CLIENT_OBJS = $(CLIENT_SRC:.c=.o)
CLIENT_TARGET = procnanny.client

BINS = $(CLIENT_TARGET) $(SERVER_TARGET)
SERVER_OBJECT_FILES = server/*.o common/*.o
CLIENT_OBJECT_FILES = client/*.i common/*.o
OTHER_OBJECT_FILES = *.o *~
OBJECT_FILES = $(OTHER_OBJECT_FILES) $(SERVER_OBJECT_FILES) $(CLIENT_OBJECT_FILES)
LOGS = $(PROCNANNYLOGS) $(PROCNANNYSERVERINFO) memwatch.log

default: all

all: client server

client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS) 

server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $<  -o $@

clean-server:
	$(RM) $(SERVER_TARGET) $(SERVER_OBJECT_FILES)

clean-client:
	$(RM) $(CLIENT_TARGET) $(CLIENT_OBJECT_FILES)

clean-all:
	$(RM) $(BINS) $(OBJECT_FILES) $(LOGS)

clean-logs:
	$(RM) $(LOGS)
	
clean:
	$(RM) $(BINS) $(OBJECT_FILES)

rebuild: clean-all all