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
{}

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

        if (message[0] == '/')
        {
            // message started with /, it should be a command
            bool shouldExit = processCommand(std::move(message));

            if (shouldExit)
                return;
        }
        else
        {
            // broadcastMessage() will lock mutex
            m_server.get().broadcastMessage(*this, std::move(message));
        }
    }
}

bool Client::processCommand(std::string message)
{
    if (message.find("quit") == 1)
    {
        // indicate that we want to disconnect
        m_wantToLeave = true;

        // returning with true, indicating that the thread should exit
        return true;
    }
    return false;
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

