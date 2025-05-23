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
    _capNegotiation = false;
    _isPasswordValid = false;
    _isNickValid = false;
    _isUSERcomplete = false;
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

void Client::setIsPasswordValid(bool isPasswordValid)
{
    _isPasswordValid = isPasswordValid;
}

void Client::setIsRegistered(bool isRegistered)
{
    _isRegistered = isRegistered;
}

bool Client::getIsPasswordValid() const
{
    return _isPasswordValid;
}

bool Client::getIsRegistered() const
{
    return _isRegistered;
}

void Client::setCapNegotiation(bool capNegotiation)
{
    _capNegotiation = capNegotiation;
}

bool Client::getCapNegotiation() const
{
    return _capNegotiation;
}

void Client::setIsNickValid(bool isNickValid)
{
    _isNickValid = isNickValid;
}

bool Client::getIsNickValid() const
{
    return _isNickValid;
}

void Client::setIsUSERcomplete(bool isUSERcomplete)
{
    _isUSERcomplete = isUSERcomplete;
}

bool Client::getIsUSERcomplete() const
{
    return _isUSERcomplete;
}

std::string Client::getUsername() const
{
    return _username;
}

std::string Client::getHostname() const
{
    return _hostname;
}

std::string Client::getServername() const
{
    return _servername;
}
