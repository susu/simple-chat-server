#include <chatserver/Client.hpp>

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chatserver/Server.hpp>

using namespace chatserver;


Client::Client(int socket, struct sockaddr_in address, Server & server)
    : m_server(server)
    , m_socket(socket)
    , m_address(address)
{
    std::cout << "Client connected (fd=" << m_socket << ", addr=" << getAddress() << ")" << std::endl;
}

Client::~Client()
{
    close(m_socket);
}

void Client::start()
{
    // Start a thread that will call the client's run() method, then move on.
    // (do not confuse std::bind() with POSIX bind()!)
    m_thread = std::thread(std::bind(&Client::run, this));
}

void Client::finish()
{
    if (m_thread.joinable())
        m_thread.join();
}

void Client::run()
{
    char buffer[1024];
    while (true)
    {
        int len = recv(m_socket, buffer, sizeof(buffer), 0);

        if (len == 0)
        {
            std::cout << "Client disconnected (fd=" << m_socket << ", addr=" << getAddress() << ")" << std::endl;
            // TODO disconnect from server!
            return;
        }
        if (len < 0)
        {
            std::cerr << "Could not receive from client: " << strerror(errno) << std::endl;
            // TODO disconnect from server!
            return;
        }

        std::string message(buffer, len);

        // broadcastMessage() will lock mutex
        m_server.get().broadcastMessage(*this, std::move(message));
    }
}

std::string Client::getAddress() const
{
    char buffer[256];
    const char * result = inet_ntop(AF_INET, &m_address.sin_addr, buffer, sizeof(buffer));

    if (result == nullptr)
        std::cerr << "Could not convert client address:" << strerror(errno) << std::endl;

    return std::string(buffer);
}

void Client::sendMessage(std::string message)
{
    int len = send(m_socket, message.c_str(), message.length(), 0);
    if (len < 0)
        std::cerr << "Could not send data: " << strerror(errno) << std::endl;
}

