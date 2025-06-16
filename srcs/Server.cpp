#include "../inc/Server.hpp"

volatile std::sig_atomic_t g_keep_running = 1;

Server::Server(int port, std::string password) : _port(port), _password(password) {
	_running = true;
	_serverName = "ircserv";
	_serverSocketFd = -1;
	_epollfd = -1;
	_nrEvents = 0;
	_clients.empty();
	_channels.empty();
	_commandMap.empty();
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
	_commandMap.insert(std::make_pair("WHO",	&Server::who));
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
    {
		if ((*i).getFd() == fd)
			break;
    }
    if (i == _clients.end())
    {
        throw std::runtime_error("Client not found");
    }
	return i;
}

std::vector<Client>::iterator	Server::getClientQUIET(int fd) {
	std::vector<Client>::iterator i = _clients.begin();
	for (; i != _clients.end(); i++)
    {
		if ((*i).getFd() == fd)
			break;
    }
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

void Server::handle_send(int client_fd)
{
    if (getClientQUIET(client_fd) == _clients.end())//client quit beforehand
        return;
    Client &client = *getClient(client_fd);
    size_t total_sent = 0;
    std::cout << "All for send(): " << client.send_buffer << "|" << std::endl;

    while (!client.send_buffer.empty()) {
        int bytes_sent = send(client.getFd(), client.send_buffer.c_str() + total_sent,
                              client.send_buffer.size() - total_sent, MSG_NOSIGNAL);
        if (bytes_sent > 0) {
            total_sent += bytes_sent;
        } else if (bytes_sent == 0) {
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else {
                std::cout << "handle_send quitting: errno=" << errno << " (" << strerror(errno) << ")" << std::endl;
                std::string x = "";
                quit(x, getClient(client_fd));
                return;
            }
        }
    }

    if (total_sent > 0)
        client.send_buffer.erase(0, total_sent);
    //remove EPOLLOUT
    if (client.send_buffer.empty()) {
        struct epoll_event ev = client.getEv();
        ev.events = EPOLLIN | EPOLLHUP | EPOLLRDHUP;
        if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, client.getFd(), &ev) == -1)
            std::cerr << "epoll_ctl(MOD) failed to disable EPOLLOUT\n";
    }
}

void Server::ft_socket()
{
    struct sockaddr_in server_addr;

    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("socket() failed");
    
    std::memset(&server_addr, 0, sizeof(server_addr));
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

    // _nfds = 1;
    // memset(_pollfds, 0, sizeof(_pollfds));
    // _pollfds[0].fd = _serverSocketFd;
    // _pollfds[0].events = POLLIN | POLLHUP | POLLOUT;
    //epoll CHANGE:
    _epollfd = epoll_create1(0);
    if (_epollfd == -1)
        throw std::runtime_error("epoll_create1() failed");
    _ev.events = EPOLLIN;
    _ev.data.fd = _serverSocketFd;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _serverSocketFd, &_ev) == -1)
        throw std::runtime_error("epoll_ctl() failed");
//    _totalnumberfds++;
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

    //epoll CHANGE:
//    if (_totalnumberfds >= SOMAXCONN)
//        throw std::runtime_error("Too many clients");
    struct epoll_event _ev;
    _ev.events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP;
    _ev.data.fd = client_fd;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_fd, &_ev) == -1)
        throw std::runtime_error("epoll_ctl() for new client failed");
    _clients.push_back(Client(hostname, _ev,_epollfd));
//    _nrEvents++;
}

void Server::recv_client(int client_fd)
{
    char buffer[4096];
    std::string parsable;

    while(parsable.find("\r\n") == std::string::npos) {
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return; // nothing to read now
            else {
                std::cerr << "recv() failed on fd " << client_fd << ": " << strerror(errno) << std::endl;
                throw std::runtime_error("recv failed");}
        } else if (bytes_received == 0) {
            std::cout << "recv_client quitting" << std::endl;
            std::string x = "";
            quit(x, getClient(client_fd));
            throw std::runtime_error("connection closed by peer");
        }
        parsable.append(buffer, bytes_received);
    }

    std::cout << "All from recv(): " << parsable << "|" << std::endl;
    parseClientInput(client_fd, parsable);
}

void Server::quit_client(int client_fd)
{
    if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, client_fd, NULL) == -1)
        std::cerr << "epoll_ctl DEL failed for fd " << client_fd << std::endl;
        
    std::vector<Client>::iterator it = getClient(client_fd);
    if (it != _clients.end())
        _clients.erase(it);
    close(client_fd);
//    _nrEvents--;
    std::cout << "Client disconnected" << std::endl;
}

bool Server::hasClient(int fd) {
	for (size_t i = 0; i < _clients.size(); ++i)
		if (_clients[i].getFd() == fd)
			return true;
	return false;
}

void signalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << " SIGINT received, shutting down server." << std::endl;}
    else if (signum == SIGQUIT) {
        std::cout << " SIGQUIT received, shutting down server." << std::endl;}
    g_keep_running = 0;
}

void Server::start()
{
    std::signal(SIGINT, signalHandler);
    std::signal(SIGQUIT, signalHandler);
	ft_socket();
	std::cout << "Socket open, awaiting clients." << std::endl;
	struct epoll_event events[SOMAXCONN];
//    int nfds;
    while (g_keep_running)
    {
        _nrEvents = epoll_wait(_epollfd, events, SOMAXCONN, -1);
        if (_nrEvents == -1 && g_keep_running)
            throw std::runtime_error("epoll_wait() failed");

        for (int i = 0; i < _nrEvents; ++i)
        {
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                if (hasClient(events[i].data.fd)) {
                    std::cout << "main loop quitting" << std::endl;
                    std::string x = "";
                    quit(x, getClient(events[i].data.fd));
                }
            }
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == _serverSocketFd)
                    accept_client();
                else if (hasClient(events[i].data.fd))
                    recv_client(events[i].data.fd);
            }
            if (events[i].events & EPOLLOUT) {
                handle_send(events[i].data.fd);
            }

//            std::cout << "this loop done with i: " << i << "--------------------------------" << std::endl;
        }
    }
    close(_serverSocketFd);
    std::cout << "Server shutting down." << std::endl;
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->getFd());
    }
    _clients.clear();
    if (_epollfd != -1) {
        close(_epollfd);
        _epollfd = -1;
    }
    std::cout << "Server shutdown complete." << std::endl;
}
