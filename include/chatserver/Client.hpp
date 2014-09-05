#ifndef CHATSERVER_CLIENT_HPP_INC
#define CHATSERVER_CLIENT_HPP_INC

#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <thread>

namespace chatserver
{
    class Server;
    class Client
    {
        public:
            Client(int socket, struct sockaddr_in address, Server & server);
            ~Client();

            // Copying is forbidden!
            Client(const Client&) = delete;
            Client& operator=(const Client&) = delete;

            // Moving is allowed.
            Client(Client&&) = default;
            Client& operator=(Client&&) = default;

            /**
             * Thread entry point
             */
            void run();

            /**
             * Starts the thread (non-blocking)
             */
            void start();

            /**
             * Wait for the thread to stop (blocking!)
             */
            void finish();

            std::string getAddress() const;

            void sendMessage(std::string message);

        private:
            /**
             * returns true if the client should be stopped
             */
            bool processCommand(std::string message);

            std::reference_wrapper<Server> m_server; // reference to the original server.
            int m_socket;
            struct sockaddr_in m_address;
            std::thread m_thread;
    };
}


#endif
