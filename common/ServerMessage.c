#include "ServerMessage.h"
#include "CommunicationManager.h"
#include <assert.h>
#include "Utils.h"
#include "memwatch.h"


bool sendMessage(ServerMessage message, int sock, LoggerPointer logger)
{
	if (writeData(sock, (char*)&(message.type), sizeof(message.type), logger) < 0)
	{
		return false;
	}

	LogReport error;
	switch (message.type)
	{
		case HUP:
			return sendConfig(sock, message.configLength, message.newConfig, logger);

		case INT:
			return true;

		default:
			error.message = "Unknown MessageType being asked to write. Only the MessageType sent";
			error.type = DEBUG;
			logger(error, false);
			return true;
	}
}

ServerMessage readMessage(int sock, LoggerPointer logger)
{
	ServerMessage message;

	if (readData(sock, (char*)&(message.type), sizeof(message.type), logger) < 0)
	{
		exit(-1);
	}

	LogReport error;
	switch (message.type)
	{
		case INT:
			break;

		case HUP:
			message.configLength = readConfig(sock, &(message.newConfig), logger);
			break;

		default:
			error.message = "Unknown MessageType read. Only the MessageType has been read. There may be some more data on the socket.";
			error.type = DEBUG;
			logger(error, false);
			break;
	}

	return message;
}