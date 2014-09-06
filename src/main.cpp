#include <chatserver/Server.hpp>

int main()
{
    chatserver::Server server(5050);

    server.run();
}
