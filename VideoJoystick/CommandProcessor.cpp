
#include "CommandProcessor.h"


CommandProcessor Commander;


CommandProcessor::CommandProcessor(void)
{
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

	std::string result = "Unknown command: \"";
	result += command;
	result += "\"";
	return result;
}

