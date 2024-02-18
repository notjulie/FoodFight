//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <functional>
#include <map>
#include <mutex>


/// <summary>
/// Command processing utility; just a collection of handlers for command lines,
/// such that you can say ProcessCommand("bleem") and it will locate and call
/// the handler for the bleem command.
/// </summary>
class CommandProcessor final {
public:
   CommandProcessor() = default;
   ~CommandProcessor() = default;


   void AddHandler(const std::string &command, const std::function<std::string (const std::string &)> &handler);

   std::string ProcessCommand(const std::string &command);

private:
   using Handler = std::function<std::string(const std::string &)>;
   std::mutex mutex;
   std::map<std::string, Handler> handlers;
};


#endif
