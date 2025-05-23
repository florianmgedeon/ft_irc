#include "Client.hpp"

Client::Client()
{
	_nickname = _username = _realname = _hostname = _servername = "*";
    _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
    _pfd = NULL;
}

Client::~Client()
{
}

Client::Client(std::string hostname, pollfd *pfd) : _hostname(hostname), _pfd(pfd)
{
	_nickname = _username = _realname = _servername = "*";
    _write_ready = false;
    _capNegotiation = false;
    _isPasswordValid = false;
    _isRegistered = false;
}

int Client::getFd() const
{
    return _pfd->fd;
}

void Client::setWrite(bool write)
{
    _write_ready = write;
}

void Client::sendToClient(std::string message)
{
//	std::cout << "appending @ client " << this->_nickname << " <" << message << ">" << std::endl;
    send_buffer += message;
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
	return (_nickname + "!" + _username + "@" + _hostname);
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
