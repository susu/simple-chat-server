#include <chatserver/Server.hpp>

#include <cassert>
#include <cstring>

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <unistd.h>

using namespace chatserver;

Server::Server(uint16_t listeningPort)
    : m_port(listeningPort)
    , m_serverSocketFd(-1)
{
}

Server::~Server()
{
    if (m_serverSocketFd != -1)
        close(m_serverSocketFd);
}

void Server::run()
{
    initializeSocket();

    while (true)
    {
        processLeavers();

        struct sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(struct sockaddr_in));

        unsigned addrlen = sizeof(struct sockaddr_in);
        int clientSocket = accept(m_serverSocketFd, (struct sockaddr*)&clientAddress, &addrlen);

        std::lock_guard<std::mutex> _lock(m_mutex);
        m_clients.emplace_back(clientSocket, clientAddress, *this);
        m_clients.back().start();
    }
}

void Server::broadcastMessage(const Client & sender, std::string message)
{
    // std::lock_guard will automatically release the mutex at the end of the scope.
    std::lock_guard<std::mutex> _lock(m_mutex);

    for (auto & client : m_clients)
    {
        std::ostringstream ss;
        ss << sender.getAddress() << ": " << message;
        client.sendMessage(ss.str());
    }

    // No need for explicit unlock/release of the mutex, lock_guard do it in its destructor.
}

void Server::processLeavers()
{
    std::lock_guard<std::mutex> _lock(m_mutex);

    for (auto it = begin(m_clients); it != end(m_clients); ++it)
    {
        if (it->IsWantToLeave())
        {
            it->finish();
            it = m_clients.erase(it);
        }
    }
}

void Server::initializeSocket()
{
    memset(&m_serverAddress, 0, sizeof(struct sockaddr_in));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_port);

    m_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    int result = bind(m_serverSocketFd, (struct sockaddr*)&m_serverAddress, sizeof(struct sockaddr_in));
    if (result < 0)
    {
        std::cerr << "Could not bind: " << strerror(errno) << std::endl;
        throw std::runtime_error("Could not bind!");
    }

    listen(m_serverSocketFd, 1);
}
