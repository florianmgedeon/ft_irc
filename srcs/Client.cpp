#include "Client.hpp"

Client::Client()
{
	_nickname = _username = _realname = _hostname = _servername = "*";
    _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
    _pfd = NULL;
	_isUserComplete = false;
	_isNickValid = false;
}

Client::~Client()
{
    std::cout << "Client destructor called for " << _nickname << std::endl;
}

Client::Client(std::string hostname, pollfd *pfd) : _hostname(hostname), _pfd(pfd)
{
	_nickname = _username = _realname = _servername = "*";
    _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
	_isUserComplete = false;
	_isNickValid = false;
}

int Client::getFd() const
{
    return _pfd->fd;
}

void Client::setWrite(bool write)
{
    _write_ready = write;
    _pfd->events |= POLLOUT;
}

void Client::sendToClient(std::string message)
{
    send_buffer += message + "\r\n";
    setWrite(true);
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
