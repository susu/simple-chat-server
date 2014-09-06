#ifndef CHATSERVER_SERVER_HPP_INC
#define CHATSERVER_SERVER_HPP_INC

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#include <netinet/in.h>

#include <chatserver/Client.hpp>

namespace chatserver
{
    class Server
    {
        public:
            Server(uint16_t listeningPort);
            ~Server();

            // Copying is forbidden!
            Server(const Server&) = delete;
            Server& operator=(const Server&) = delete;

            // Moving is forbidden!.
            Server(Server&&) = delete;
            Server& operator=(Server&&) = delete;

            /**
             *  Main entry point
             */
            void run();

            void broadcastMessage(const Client& sender, std::string message);
            void disconnectClient(const Client * client);

        private:
            void initializeSocket();
            std::string createUniqueId(const struct sockaddr_in & clientAddress);

            uint16_t m_port;
            int m_serverSocketFd;
            struct sockaddr_in m_serverAddress;

            std::mutex m_mutex;
            std::vector<std::shared_ptr<Client>> m_clients;
            int m_nextClientId;
    };
}

#endif
