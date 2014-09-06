#include <chatserver/Client.hpp>

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <chatserver/Server.hpp>

using namespace chatserver;


Client::Client(std::string name, int socket, struct sockaddr_in address, Server & server)
    : m_name(name)
    , m_server(server)
    , m_socket(socket)
{
    std::cout << "Client connected (fd=" << m_socket << ", addr=" << getName() << ")" << std::endl;
}

Client::~Client()
{
    close(m_socket);
}

void Client::start()
{
    // Start a thread that will call the client's run() method, then move on.
    // (do not confuse std::bind() with POSIX bind()!)
    m_thread = std::thread(std::bind(&Client::run, shared_from_this()));
}

void Client::run()
{
    char buffer[1024];
    while (true)
    {
        int len = recv(m_socket, buffer, sizeof(buffer), 0);

        if (len == 0)
        {
            std::cout << "Client disconnected (fd=" << m_socket << ", addr=" << getName() << ")" << std::endl;
            break;
        }
        if (len < 0)
        {
            std::cerr << "Could not receive from client: " << strerror(errno) << std::endl;
            break;
        }

        std::string message(buffer, len);

        // broadcastMessage() will lock mutex
        m_server.get().broadcastMessage(*this, std::move(message));
    }
    m_server.get().disconnectClient(this);
    m_thread.detach();
}

void Client::sendMessage(std::string message)
{
    int len = send(m_socket, message.c_str(), message.length(), 0);
    if (len < 0)
        std::cerr << "Could not send data: " << strerror(errno) << std::endl;
}

