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

class Server
{
    private:
        int         _port;
        std::string _password;
        pollfd     _pollfds[SOMAXCONN];
        nfds_t      _nfds;

    public:
        Server(int port, std::string password);
        ~Server();
        int         getPort() const;
        std::string getPassword() const;
        void        start();
        void        ft_socket();

};

#endif