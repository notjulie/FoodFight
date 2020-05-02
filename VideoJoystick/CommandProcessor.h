
#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <functional>
#include <map>
#include <mutex>


class CommandProcessor {
public:
   CommandProcessor(void);

   std::string ProcessCommand(const std::string &command);

private:
   std::mutex mutex;
   std::map<std::string, std::function<std::string(const std::string &)>> handlers;
};


extern CommandProcessor Commander;


#endif
