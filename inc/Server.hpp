#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <map>
#include "Client.hpp"

class Server
{
    private:
        int                     _port;
        std::string             _password;
        pollfd                  _pollfds[SOMAXCONN];
        nfds_t                  _nfds;
        int                     _serverSocketFd;
        bool                    _running;
        std::map<int, Client>   _clients;

    public:
        Server(int port, std::string password);
        ~Server();
        int         getPort() const;
        std::string getPassword() const;
        void        setRunning(bool running);
        void        start();
        void        ft_socket();
        void        accept_client();


};

#endif