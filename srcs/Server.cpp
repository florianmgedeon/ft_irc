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
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //fcntl

}

void Server::start()
{
    ft_socket();
}