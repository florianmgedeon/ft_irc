#include "Client.hpp"

Client::Client()
{
	_nickname = _username = _realname = _hostname = _servername = "*";
    // _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
	_isUserComplete = false;
	_isNickValid = false;
}

Client::~Client()
{
    std::cout << "Client destructor called for " << _nickname << std::endl;
}

// Client::Client(std::string hostname, pollfd *pfd) : _hostname(hostname), _pfd(pfd)
Client::Client(std::string hostname, struct epoll_event _ev) : _hostname(hostname), _ev(_ev)
{
	_nickname = _username = _realname = _servername = "*";
    // _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
	_isUserComplete = false;
	_isNickValid = false;
}

int Client::getFd() const
{
    return _ev.data.fd;
}

// void Client::setWrite(bool write)
// {
//     // _write_ready = write;
//     //NOT NEEDED ANYMORE BECUASE OF EPOLLET at the start:
//     // if (write == true)
//     // {
//     //     _ev.events |= EPOLLOUT;
//     //     if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _ev.data.fd, &_ev) == -1)
//     //         throw std::runtime_error("epoll_ctl(MOD) failed");
//     // }
//     // else
//     // {
//     //     _ev.events = EPOLLIN | EPOLLET | EPOLLHUP;
//     //     if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _ev.data.fd, &_ev) == -1)
//     //         throw std::runtime_error("epoll_ctl(MOD) failed");
//     // }
// }

void	Client::sendOff() {
	send(_ev.data.fd, send_buffer.c_str(), send_buffer.size(), 0);
	send_buffer.erase();
}

void Client::sendToClient(std::string message)
{
    send_buffer += message + "\r\n";
    // setWrite(true);
}

void Client::recvFromClient(char *buffer)
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

std::string Client::getColNick(void) {
	return (":" + _nickname);
}

std::string Client::getNickUserHost(void) {
	return (":" + _nickname + "!" + _username + "@" + _hostname);
}

std::string Client::getColHost(void) {
	return (":" + _hostname);
}

bool Client::getIsRegistered() const
{
    return _isRegistered;
}

void Client::setIsRegistered(bool isRegistered)
{
    _isRegistered = isRegistered;
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

void Client::setCapNegotiation(bool capNegotiation)
{
    _capNegotiation = capNegotiation;
}
bool Client::isIsPasswordValid() const {
	return _isPasswordValid;
}

void Client::setIsPasswordValid(bool isPasswordValid) {
	_isPasswordValid = isPasswordValid;
}

bool Client::getIsPasswordValid() const {
	return _isPasswordValid;
}

bool Client::getCapNegotiation() const
{
    return _capNegotiation;
}

void Client::setIsNickValid(bool isNickValid) {
    _isNickValid = isNickValid;
}

bool Client::getIsNickValid() const {
    return _isNickValid;
}

void Client::setIsUserComplete(bool isUserComplete) {
    _isUserComplete = isUserComplete;
}

bool Client::getIsUserComplete() const {
    return _isUserComplete;
}

std::string Client::getUsername(void) const
{
    return _username;
}

std::string Client::getHostname(void) const
{
    return _hostname;
}

std::string Client::getServername(void) const
{
    return _servername;
}
