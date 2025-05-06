#include "../inc/Server.hpp"

int checkPort(char *av)
{
    long port = strtol(av, NULL, 0);
    if (port < 1024 || port > 65535)
    {
        throw std::invalid_argument("Port must be between 1024 and 65535");
    }
    return static_cast<int>(port);
}

int main (int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    //server constructor
    try
    {
        int port = checkPort(av[1]);
        Server server(port, av[2]);
        std::cout << server.getPort() << std::endl;
        std::cout << server.getPassword() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}