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

int Client::getFd() const
{
    return _fd;
}

void Client::setWrite(bool write)
{
    write_ready = write;
}

//append_send_buffer
void Client::append_send_buffer(std::string message)
{
    send_buffer += message;
}