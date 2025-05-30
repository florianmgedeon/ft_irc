#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password) {
	_running = true;
	_serverName = "ircserv";
	_serverSocketFd = -1;
	_nfds = 0;
	_commandMap.insert(std::make_pair("CAP",	&Server::cap));
	_commandMap.insert(std::make_pair("INVITE",	&Server::invite));
	_commandMap.insert(std::make_pair("JOIN",	&Server::join));
	_commandMap.insert(std::make_pair("KICK",	&Server::kick));
	_commandMap.insert(std::make_pair("MODE",	&Server::mode));
	_commandMap.insert(std::make_pair("NAMES",	&Server::names));
	_commandMap.insert(std::make_pair("NICK",	&Server::nick));
	_commandMap.insert(std::make_pair("NOTICE",	&Server::notice));
	_commandMap.insert(std::make_pair("PART",	&Server::part));
	_commandMap.insert(std::make_pair("PASS",	&Server::pass));
	_commandMap.insert(std::make_pair("PING",	&Server::ping));
	_commandMap.insert(std::make_pair("PONG",	&Server::pong));
	_commandMap.insert(std::make_pair("PRIVMSG",&Server::privmsg));
	_commandMap.insert(std::make_pair("TOPIC",	&Server::topic));
	_commandMap.insert(std::make_pair("USER",	&Server::user));
	_commandMap.insert(std::make_pair("QUIT",	&Server::quit));
}

Server::~Server() {}

int Server::getPort() const						{return _port;}
const std::string &Server::getPassword() const	{return _password;}
void Server::setRunning(bool running)			{_running = running;}
const std::string& Server::getServerName() const {return _serverName;}
void Server::setServerName(const std::string &serverName) {_serverName = serverName;}

std::vector<Client>::iterator	Server::getClient(int fd) {
	std::vector<Client>::iterator i = _clients.begin();
	for (; i != _clients.end(); i++)
		if ((*i).getFd() == fd)
			break;
	return i;
}

std::vector<Client>::iterator	Server::getClient(const std::string nickname) {
	std::vector<Client>::iterator i = _clients.begin();
	for (; i != _clients.end(); i++) {
		std::string temp = (*i).getNickname();
		if (!temp.compare(nickname.c_str()))
			break;
	}
	return i;
}

bool	Server::channelExists(std::string nick) {
	return _channels.find(nick) == _channels.end() ? false : true;
}

//============================================ poll loop ====================================//

void Server::handle_send(int index)
{
    Client &client = *(getClient(_pollfds[index].fd));
    if (!client.send_buffer.empty())
    {
        int bytes_sent = send(client.getFd(), client.send_buffer.c_str(), client.send_buffer.size(), MSG_NOSIGNAL);
        if (bytes_sent > 0)
        {
            std::cout << "buffer from send: " << client.send_buffer.c_str() << std::endl;
            client.send_buffer.erase(0, bytes_sent);
            if (client.send_buffer.empty())
            {
                client.setWrite(false);
                _pollfds[index].events = POLLIN | POLLHUP;
            }
        }
        else if (bytes_sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            quit_client(index);
    }
}

void Server::ft_socket()
{
    struct sockaddr_in server_addr;

    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("socket() failed");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_port = htons(_port); //port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //IP address


    int opt = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt() failed");

    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");

    if (bind(_serverSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        throw std::runtime_error("bind() failed");

    if (listen(_serverSocketFd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");

    _nfds = 1;
    memset(_pollfds, 0, sizeof(_pollfds));
    _pollfds[0].fd = _serverSocketFd;
    _pollfds[0].events = POLLIN | POLLHUP | POLLOUT;
}

void Server::accept_client()
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(_serverSocketFd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd == -1)
        throw std::runtime_error("accept() failed");
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    std::string hostname(inet_ntoa(client_addr.sin_addr));

    if (_nfds >= SOMAXCONN)
        throw std::runtime_error("Too many clients");
    _pollfds[_nfds].fd = client_fd;
    _pollfds[_nfds].events = POLLIN | POLLHUP;
    _clients.push_back(Client(hostname, &_pollfds[_nfds]));
    _nfds++;
}

bool Server::recv_client(int index)
{
    char buffer[512];
    int client_fd = _pollfds[index].fd;
    // Client &client = _clients[client_fd];
    // if (!client.getIsRegistered())
    //     return false;
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received == 0)
        return quit_client(index); // true = client removed
    else if (bytes_received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        throw std::runtime_error("recv() failed");
    }
    else
    {
        std::cout << "buffer from recv: " << buffer << std::endl;
        parseClientInput(client_fd, std::string(buffer));
        return false;
    }
}

bool Server::quit_client(int index)
{
    std::vector<Client>::iterator clientIt = getClient(_pollfds[index].fd);
    close(_pollfds[index].fd);

    if (_nfds > 2)
    {
        _pollfds[index] = _pollfds[_nfds - 1];
        _pollfds[_nfds - 1].fd = -1;
        _pollfds[_nfds - 1].events = 0;
    }
    else if (_nfds == 2)
    {
        _pollfds[index].fd = -1;
        _pollfds[index].events = 0;
    }

    _nfds--;
    _clients.erase(clientIt);
    std::cout << "Client disconnected" << std::endl;
    return true;
}

void Server::start()
{
    ft_socket();
    std::cout << "Socket open, awaiting clients." << std::endl;
    while (_running)
    {
        if (poll(_pollfds, _nfds, -1) == -1)
            throw std::runtime_error("poll() failed");
        if (_pollfds[0].revents & POLLIN)
            accept_client();
        nfds_t i = 1;
        while (i < _nfds)
        {
            short revents = _pollfds[i].revents;
            bool client_removed = false;
            try
            {
                if (revents & POLLIN)
                    client_removed = recv_client(i);
                if (!client_removed && (revents & POLLOUT))
                    handle_send(i);
                if (!client_removed && (revents & POLLHUP))
                    client_removed = quit_client(i);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error in poll loop: " << e.what() << std::endl;
                client_removed = quit_client(i);
            }
            if (!client_removed)
                ++i;
        }
        //std::cout << "Number of clients: " << (_nfds - 1) << std::endl;
    }
    close(_serverSocketFd);
}
