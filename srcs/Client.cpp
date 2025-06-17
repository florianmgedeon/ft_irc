#include "Client.hpp"

Client::Client()
{
	_nickname = _username = _realname = _hostname = _servername = "*";
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
	_isUserComplete = false;
	_isNickValid = false;
	_epollfd = -1;
}

Client::~Client()
{
}

Client::Client(std::string hostname, struct epoll_event _ev, int epollfd) : _hostname(hostname), _epollfd(epollfd), _ev(_ev) {
	_nickname = _username = _realname = _servername = "*";
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

struct epoll_event &Client::getEv()
{
    return _ev;
}

void Client::sendToClient(std::string message)
{
    send_buffer += message + "\r\n";
    _ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP;
    if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _ev.data.fd, &_ev) == -1)
        throw std::runtime_error("epoll_ctl(MOD) failed");
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
