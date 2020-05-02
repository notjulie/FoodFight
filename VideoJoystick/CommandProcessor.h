
#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <functional>
#include <map>
#include <mutex>


class CommandProcessor {
public:
   CommandProcessor(void);
   void AddHandler(const std::string &command, const std::function<std::string (const std::string &)> &handler);

   std::string ProcessCommand(const std::string &command);

private:
   using Handler = std::function<std::string(const std::string &)>;
   std::mutex mutex;
   std::map<std::string, Handler> handlers;
};


extern CommandProcessor Commander;


#endif
