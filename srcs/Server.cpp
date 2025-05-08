#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password)
{   
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

void Server::ft_socket()//take port as arg and not as class member?
{
    struct sockaddr_in server_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        throw std::runtime_error("socket() failed");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_port = htons(this->_port); //port
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP address

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt() failed");

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        throw std::runtime_error("bind() failed");

    if (listen(sockfd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");

    _nfds = 1;
    memset(_pollfds, 0, sizeof(_pollfds));
    _pollfds[0].fd = sockfd;
    _pollfds[0].events = POLLIN | POLLOUT | POLLOUT;
}

void Server::start()
{
    ft_socket();
}