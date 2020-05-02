
#include "CommandProcessor.h"


CommandProcessor Commander;


CommandProcessor::CommandProcessor(void)
{
}


void CommandProcessor::AddHandler(const std::string &command, const std::function<void()> &handler)
{
   AddHandler(command, [handler](const std::string &params) {
	  handler();
	  return "";
   });
}

void CommandProcessor::AddHandler(const std::string &command, const std::function<std::string (const std::string &)> &handler)
{
   std::lock_guard<std::mutex> lock(mutex);
   handlers[command] = handler;
}


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

