#ifndef CHATSERVER_CLIENT_HPP_INC
#define CHATSERVER_CLIENT_HPP_INC

#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <thread>

namespace chatserver
{
    class Server;
    class Client : public std::enable_shared_from_this<Client>
    {
        public:
            Client(std::string name, int socket, struct sockaddr_in address, Server & server);
            ~Client();

            // Copying is forbidden!
            Client(const Client&) = delete;
            Client& operator=(const Client&) = delete;

            // Moving is forbidden!
            Client(Client&&) = delete;
            Client& operator=(Client&&) = delete;

            /**
             * Thread entry point
             */
            void run();

            /**
             * Starts the thread (non-blocking)
             */
            void start();

            std::string getName() const
            { return m_name; }

            void sendMessage(std::string message);

        private:
            std::string m_name;
            std::reference_wrapper<Server> m_server; // reference to the original server.
            int m_socket;
            std::thread m_thread;
    };
}


#endif
