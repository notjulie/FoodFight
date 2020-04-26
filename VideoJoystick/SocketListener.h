
#ifndef TEMPESTSOCKETLISTENER_H
#define TEMPESTSOCKETLISTENER_H

#include <functional>
#include <mutex>
#include <vector>
#include <thread>

class SocketListener {
   public:
      SocketListener(const std::function<std::string(const std::string &)> &handler);
      ~SocketListener(void);

   private:
      void ListenToClient(int client);
      void ListenForConnections(void);

   private:
      bool terminated;
      int theSocket;
      std::mutex clientListMutex;
      std::thread *connectionThread;
      std::vector<std::thread *> clientThreads;
      std::function<std::string(const std::string &)> handler;
};


#endif

