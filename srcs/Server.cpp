#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password)
{   
    _running = true;
}

Server::~Server()
{
}

int Server::getPort() const
{
    return _port;
}

std::string Server::getPassword() const
{
    return _password;
}

void Server::setRunning(bool running)
{
    _running = running;
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
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP address

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
    _pollfds[0].events = POLLIN | POLLOUT | POLLOUT;
}

void Server::accept_client()
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(_serverSocketFd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd == -1)
        throw std::runtime_error("accept() failed");
    std::string hostname(inet_ntoa(client_addr.sin_addr));
    _clients[client_fd] = Client(hostname, client_fd);
    if (_nfds >= SOMAXCONN)
        throw std::runtime_error("Too many clients");
    _pollfds[_nfds].fd = client_fd;
    _pollfds[_nfds].events = POLLIN;//more flags needed?
    _nfds++;
}

void Server::start()
{
    ft_socket();
    while(_running)
    {
        if (poll(_pollfds, _nfds, -1) == -1)
            throw std::runtime_error("poll() failed");
        for (int i = 0; i < _nfds; ++i)
        {
            if (_pollfds[i].revents & POLLIN)//new info to read
            {
                if (_pollfds[i].fd == _serverSocketFd)//from server
                    accept_client();
                else//from client
                    recv_client(i);
            }
            else if ((_pollfds[i].revents & POLLHUP) || (_pollfds[i].revents & POLLOUT)) //pollout???
                quit_client(i);
        }
    }
    close(_serverSocketFd);
}