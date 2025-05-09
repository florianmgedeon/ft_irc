#include "Client.hpp"

Client::Client()
{
}

Client::~Client()
{
}

Client::Client(std::string hostname, int fd) : _hostname(hostname), _fd(fd)
{
}