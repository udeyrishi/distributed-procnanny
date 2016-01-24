/*
Copyright 2015 Udey Rishi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ServerMessage.h"
#include "CommunicationManager.h"
#include "Utils.h"
#include <assert.h>
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