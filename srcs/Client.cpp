#include "Client.hpp"

Client::Client()
{
}

Client::~Client()
{
}

Client::Client(std::string hostname, int fd) : _hostname(hostname), _fd(fd)
{
    _write_ready = false;
    _isPasswordValid = false;
}

int Client::getFd() const
{
    return _fd;
}

void Client::setWrite(bool write)
{
    _write_ready = write;
}

//append_send_buffer
void Client::append_send_buffer(std::string message)
{
    send_buffer += message;
}