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
#include "Channel.hpp"
#include "Command.hpp"
#include <cerrno>

class Channel;
class Command;
class Client;

class Server
{
    private:
        int                             _port;
        std::string                     _password;
        pollfd                          _pollfds[SOMAXCONN];
        nfds_t                          _nfds;
        int                             _serverSocketFd;
        bool                            _running;
        std::map<int, Client>           _clients;
        std::map<std::string, Channel>  _channels;
        std::string                     _serverName;

    public:
        Server(int port, std::string password);
        ~Server();
        int         getPort() const;
        std::string getPassword() const;
        void        setRunning(bool running);
        void        start();
        void        ft_socket();
        void        accept_client();
        bool        recv_client(int index);
        bool        quit_client(int index);
        void        ft_send(int fd, const std::string message);
        void        handle_send(int index);
        void        create_command(int fd, char *buffer);
        void        find_command(Command command);
        void        numeric_reply(int fd, const std::string& code, const std::string& target, const std::string& msg);

        void        cap_command(Command command);
        void        pass_command(Command command);
        void        nick_command(Command command);
        void        user_command(Command command);
        // void        ping_command(Command command);
        // void        pong_command(Command command);
        // void        kick_command(Command command);
        // void        invite_command(Command command);
        // void        topic_command(Command command);
        // void        mode_command(Command command);
};

#endif