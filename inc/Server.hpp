#pragma once

#include <iostream>
#include <sstream>
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
#include <vector>
#include "Client.hpp"
#include "Channel.hpp"
#include <cerrno>
#include <bits/stdc++.h>

class Channel;
class Client;

class Server
{
    private:
        //boilerplate for server function
        typedef bool(Server::*cmd_t)(std::string&, Client&);
        typedef std::map<std::string, cmd_t> commandMap_t;
        typedef commandMap_t::iterator  commandIter;
        int                             _port;
        std::string                     _password;
        pollfd                          _pollfds[SOMAXCONN];
        nfds_t                          _nfds;
        int                             _serverSocketFd;
        bool                            _running;
        std::vector<Client>             _clients;
        std::map<std::string, Channel>  _channels;
        std::string                     _serverName;
        commandMap_t                    _commandMap;

    public:
        Server(int port, std::string password);
        ~Server();
        int         getPort() const;
        const std::string &getPassword() const;
        void        setRunning(bool running);
        const std::string& getServerName() const;
        void setServerName(const std::string &serverName);
        std::vector<Client>::iterator getClient(int fd);
        std::vector<Client>::iterator getClient(const std::string nickname);
        int&		getClientFd(Client &c, int fd);
        void        start();
        void        ft_socket();

        void        accept_client();
        bool        recv_client(int index);
        bool        quit_client(int index);
        void        handle_send(int index);

//Parsing and commands
        bool        parseClientInput(int fd, std::string buffer);

        bool		cap(std::string &line, Client &c);
        bool		invite(std::string &line, Client &c);
        bool		join(std::string &line, Client &c);
        bool		kick(std::string &line, Client &c);
        bool		list(std::string &line, Client &c);
        bool		mode(std::string &line, Client &c);
        bool		names(std::string &line, Client &c);
        bool		nick(std::string &line, Client &c);
        bool		part(std::string &line, Client &c);
        bool		pass(std::string &line, Client &c);
        bool		ping(std::string &line, Client &c);
        bool		pong(std::string &line, Client &c);
        bool		privmsg(std::string &line, Client &c);
        bool		topic(std::string &line, Client &c);
        bool		user(std::string &line, Client &c);
};
