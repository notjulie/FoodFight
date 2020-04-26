
#include "SocketListener.h"

#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>



SocketListener::SocketListener(const std::function<std::string(const std::string &)> &handler)
{
   // copy parameters
   this->handler = handler;

   // clear
   terminated = false;
   theSocket = -1;

   // start our listener
   connectionThread = new std::thread(
      [this]() { ListenForConnections(); }
      );
}


SocketListener::~SocketListener(void)
{
   // signal that we are terminating
   terminated = true;

   // shutdown the socket
   if (theSocket != -1)
   {
      shutdown(theSocket, SHUT_RDWR);
      close(theSocket);
   }

   // destroy threads
   if (connectionThread != NULL)
   {
      connectionThread->join();
      delete connectionThread;
   }
   for (unsigned i=0; i<clientThreads.size(); ++i)
   {
      clientThreads[i]->join();
      delete clientThreads[i];
   }
}

void SocketListener::ListenForConnections(void)
{
   bool bound = false;

   while (!terminated)
   {
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
            // launch a thread to run the client
            std::thread *clientThread = new std::thread(
               [=]() { ListenToClient(newConnection); }
               );

            // add to the list
            std::lock_guard<std::mutex> lock(clientListMutex);
            clientThreads.push_back(clientThread);
         }
      }
   }
}

void SocketListener::ListenToClient(int client)
{
   std::string command;

   while (!terminated)
   {
      // read from the connection
      uint8_t b;
      int recvResult = recv(client, &b, 1, MSG_DONTWAIT);

      // a result of 1 means we got something
      if (recvResult == 1)
      {
         switch (b)
         {
         case '\n':
            {
               std::string response = handler(command);
               response += "\r\n";
               send(client, response.c_str(), response.size(), 0);
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
         continue;
      }
      else if (recvResult == 0)
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

   shutdown(client, SHUT_RDWR);
   close(client);
}
