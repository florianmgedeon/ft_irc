#include "Client.hpp"

Client::Client()
{
}

Client::~Client()
{
}

Client::Client(std::string hostname, pollfd *pfd) : _hostname(hostname), pfd(pfd)
{
    _write_ready = false;
    _isPasswordValid = false;
    _isRegistered = false;
}

int Client::getFd() const
{
    return pfd->fd;
}

void Client::setWrite(bool write)
{
    _write_ready = write;
}

void Client::append_send_buffer(std::string message)
{
    send_buffer += message;
}

void Client::append_recv_buffer(char *buffer)
{
    recv_buffer += buffer;
}

void Client::setNickname(std::string nickname)
{
    _nickname = nickname;
}

std::string &Client::getNickname(void)
{
    return _nickname;
}

bool Client::getIsRegistered() const
{
    return _isRegistered;
}

void Client::setUsername(std::string username)
{
    _username = username;
}

void Client::setHostname(std::string hostname)
{
    _hostname = hostname;
}

void Client::setServername(std::string servername)
{
    _servername = servername;
}

void Client::setRealname(std::string realname)
{
    _realname = realname;
}

void Client::setIsRegistered(bool isRegistered)
{
    _isRegistered = isRegistered;
}

