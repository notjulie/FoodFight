//
// Author: Randy Rasmussen
// Copyright: none, use as you will
// Warantee: none, your own risk
//

#include "SocketListener.h"

#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


// =======================================================
//   class SocketListener
// =======================================================

/// <summary>
/// Initializes a new instance of class SocketListener
/// </summary>
SocketListener::SocketListener(const std::function<std::string(const std::string &)> &handler)
{
   // copy parameters
   this->handler = handler;

   // start our listener
   connectionThread = new std::thread(
      [this]() { ListenForConnections(); }
      );
}


/// <summary>
/// Releases resources held by the object
/// </summary>
SocketListener::~SocketListener()
{
   // signal that we are terminating
   terminated = true;

   // shutdown the socket
   if (theSocket != -1)
   {
      shutdown(theSocket, SHUT_RDWR);
      close(theSocket);
   }

   // destroy our thread
   if (connectionThread != NULL)
   {
      connectionThread->join();
      delete connectionThread;
   }

   // destroy an open connections
   for (auto connection : connections)
      delete connection;
}


/// <summary>
/// our listener thread; listens to incoming connection requests and
/// aceepts them
/// </summary>
void SocketListener::ListenForConnections(void)
{
   bool bound = false;

   while (!terminated)
   {
      // clear out any dead connections from our list
      {
         std::lock_guard<std::mutex> lock(clientListMutex);
         size_t i = 0;
         while (i < connections.size())
         {
            if (connections[i]->IsActive())
            {
               ++i;
            }
            else
            {
               delete connections[i];
               connections[i] = connections[connections.size() - 1];
               connections.pop_back();
            }
         }
      }

      // create the socket if necessary
      if (theSocket == -1)
      {
         theSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
         if (theSocket == -1)
         {
            sleep(1);
            continue;
         }
      }

      // bind
      if (!bound)
      {
         sockaddr_in address;
         std::memset(&address, 0, sizeof(address));
         address.sin_family = AF_INET;
         address.sin_addr.s_addr = htonl(INADDR_ANY);
         address.sin_port = htons(4242);
         int result = bind(theSocket, (sockaddr *)&address, sizeof(address));
         bound = result != -1;
         if (!bound)
         {
            sleep(1);
            continue;
         }
      }

      // listen... see if we have a new connection
      if (listen(theSocket, 1) == 0)
      {
         int newConnection = accept(theSocket, NULL, NULL);
         if (newConnection != -1)
         {
            // create a connection and add it to the list
            std::lock_guard<std::mutex> lock(clientListMutex);
            connections.push_back(new SocketListenerConnection(newConnection, handler));
         }
      }
   }
}


// =======================================================
//   class SocketListenerConnection
// =======================================================


/// <summary>
/// Initializes a new instance of SocketListenerConnection
/// </summary>
SocketListenerConnection::SocketListenerConnection(int clientSocket, const std::function<std::string(const std::string &)> &handler)
{
   // copy parameters
   this->clientSocket = clientSocket;
   this->handler = handler;

   // start our listener
   listenThread = new std::thread(
      [this]() {
         ListenToClient();
         active = false;
      }
      );
}


/// <summary>
/// Releases resources held by the object
/// </summary>
SocketListenerConnection::~SocketListenerConnection()
{
   this->terminated = true;
   this->listenThread->join();
}


/// <summary>
/// listens to incoming commands and processes them
/// </summary>
void SocketListenerConnection::ListenToClient()
{
   std::string command;

   while (!terminated)
   {
      // read from the connection
      uint8_t b;
      int recvResult = recv(clientSocket, &b, 1, MSG_DONTWAIT);

      // a result of 1 means we got something
      if (recvResult == 1)
      {
         switch (b)
         {
         case '\n':
            {
               std::string response = handler(command);
               response += "\r\n";
               send(clientSocket, response.c_str(), response.size(), 0);
               command = std::string();
               break;
            }

         case '\r':
            break;

         default:
            command += (char)b;
            break;
         }
         continue;
      }

      // a result of zero means an orderly shutdown of the socket
      if (recvResult == 0)
      {
         break;
      }
      else if (recvResult == -1)
      {
         if (errno == EAGAIN)
         {
            usleep(100);
            continue;
         }
      }

      // unexpected result
      break;
   }

   shutdown(clientSocket, SHUT_RDWR);
   close(clientSocket);
}
