#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password)
{   
}

Server::~Server()
{
}

int Server::getPort() const
{
    return _port;
}

std::string Server::getPassword() const
{
    return _password;
}