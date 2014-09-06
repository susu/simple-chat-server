#include <chatserver/Server.hpp>

#include <cassert>
#include <cstring>

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>

using namespace chatserver;

Server::Server(uint16_t listeningPort)
    : m_port(listeningPort)
    , m_serverSocketFd(-1)
    , m_nextClientId(0)
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
        struct sockaddr_in clientAddress;
        memset(&clientAddress, 0, sizeof(struct sockaddr_in));

        unsigned addrlen = sizeof(struct sockaddr_in);
        int clientSocket = accept(m_serverSocketFd, (struct sockaddr*)&clientAddress, &addrlen);

        std::lock_guard<std::mutex> _lock(m_mutex);

        auto client =
            std::shared_ptr<Client>(new Client(createUniqueId(clientAddress), clientSocket, clientAddress, *this));
        m_clients.push_back(std::move(client));
        m_clients.back()->start();
    }
}

void Server::disconnectClient(const Client * client)
{
    std::lock_guard<std::mutex> _lock(m_mutex);

    m_clients.erase(std::remove_if(begin(m_clients), end(m_clients),
    [client](const auto & i)
    {
        return (i.get() == client);
    }));
}

void Server::broadcastMessage(const Client & sender, std::string message)
{
    // std::lock_guard will automatically release the mutex at the end of the scope.
    std::lock_guard<std::mutex> _lock(m_mutex);

    for (auto & client : m_clients)
    {
        std::ostringstream ss;
        ss << sender.getName() << ": " << message;
        client->sendMessage(ss.str());
    }

    // No need for explicit unlock/release of the mutex, lock_guard do it in its destructor.
}

std::string Server::createUniqueId(const struct sockaddr_in & address)
{
    char buffer[256];
    const char * result = inet_ntop(AF_INET, &address.sin_addr, buffer, sizeof(buffer));

    if (result == nullptr)
        std::cerr << "Could not convert client address:" << strerror(errno) << std::endl;

    std::ostringstream out;
    out << m_nextClientId++ << "-" << result << ":" << address.sin_port;
    return out.str();
}

void Server::initializeSocket()
{
    memset(&m_serverAddress, 0, sizeof(struct sockaddr_in));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;
    m_serverAddress.sin_port = htons(m_port);

    m_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    // Let bind to reuse previously bound adress.
    int val = 1;
    setsockopt(m_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    int result = bind(m_serverSocketFd, (struct sockaddr*)&m_serverAddress, sizeof(struct sockaddr_in));
    if (result < 0)
    {
        std::cerr << "Could not bind: " << strerror(errno) << std::endl;
        throw std::runtime_error("Could not bind!");
    }

    listen(m_serverSocketFd, 1);
}
