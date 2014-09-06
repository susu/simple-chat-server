#ifndef CHATSERVER_SERVER_HPP_INC
#define CHATSERVER_SERVER_HPP_INC

#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

#include <netinet/in.h>

#include <chatserver/Client.hpp>

namespace chatserver
{
    class Server
    {
        public:
            Server(uint16_t listeningPort);
            ~Server();

            /**
             *  Main entry point
             */
            void run();

            void broadcastMessage(const Client& sender, std::string message);
            void processLeavers();

        private:
            void initializeSocket();

            uint16_t m_port;
            int m_serverSocketFd;
            struct sockaddr_in m_serverAddress;

            std::mutex m_mutex;
            std::vector<Client> m_clients;
    };
}

#endif
