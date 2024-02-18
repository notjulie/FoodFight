//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include "CommandProcessor.h"


/// <summary>
/// Adds a handler for the given command to our list of handlers; overwrites
/// any existing handler for that command
/// </summary>
void CommandProcessor::AddHandler(const std::string &command, const std::function<std::string (const std::string &)> &handler)
{
   std::lock_guard<std::mutex> lock(mutex);
   handlers[command] = handler;
}


/// <summary>
/// Processes the given command, returning a string result.  For a command line
/// of "bleem 42 7", the handler for "bleem" is located and called with the
/// parameter "42 7".
/// </summary>
std::string CommandProcessor::ProcessCommand(const std::string &_command)
{
	// split the command into a command and a parameter string
	std::string command;
	std::string parameters;
	auto spacePosition = _command.find(' ');
	if (spacePosition == std::string::npos)
	{
		command = _command;
	}
	else
	{
		command.append(_command.begin(), _command.begin() + spacePosition);
		parameters.append(_command.begin() + spacePosition + 1, _command.end());
	}

	// find the handler
	Handler handler;
	bool found = false;
	{
		std::lock_guard<std::mutex> lock(mutex);
		auto hi = handlers.find(command);
		if (hi != handlers.end())
		{
			found = true;
			handler = (*hi).second;
		}
	}

	// execute if found
	if (found)
	{
		return handler(parameters);
	}
	else
	{
		std::string result = "Unknown command: \"";
		result += command;
		result += "\"";
		return result;
	}
}

