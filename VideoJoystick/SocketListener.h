//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include <functional>
#include <mutex>
#include <vector>
#include <thread>

class SocketListenerConnection;


/// <summary>
/// A listener on a connection-based TCP socket that accepts connections
/// that pass text-based commands to a handler function
/// <summary>
class SocketListener final
{
public:
   SocketListener(const std::function<std::string(const std::string &)> &handler);
   ~SocketListener();

private:
   void ListenForConnections();

private:
   bool terminated = false;
   int theSocket = -1;
   std::mutex clientListMutex;
   std::thread *connectionThread = nullptr;
   std::vector<SocketListenerConnection *> connections;
   std::function<std::string(const std::string &)> handler;
};


/// <summary>
/// A connection accepted by our socket listener
/// <summary>
class SocketListenerConnection final
{
public:
   SocketListenerConnection(int clientSocket, const std::function<std::string(const std::string &)> &handler);
   ~SocketListenerConnection();

   bool IsActive() const { return active; }

private:
   void ListenToClient();

private:
   std::thread *listenThread = nullptr;
   bool active = true;
   bool terminated = false;
   int clientSocket = -1;
   std::function<std::string(const std::string &)> handler;
};

#endif

