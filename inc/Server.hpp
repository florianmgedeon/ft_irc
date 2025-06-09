#pragma once

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
// #include <poll.h>
#include <sys/epoll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "Channel.hpp"
#include "Client.hpp"

class Channel;
class Client;

class Server
{
    private:
        //boilerplate for server function
        typedef bool(Server::*cmd_t)(std::string&, std::vector<Client>::iterator);
        typedef std::map<std::string, cmd_t> commandMap_t;
        typedef commandMap_t::iterator  commandIter;
        int                             _serverSocketFd;
        int                             _port;
        std::string                     _password;
        // pollfd                          _pollfds[SOMAXCONN];
        int                             _epollfd;
        struct epoll_event              _ev;
        int                             _nrEvents;
        // nfds_t                          _nfds;
        bool                            _running;
        std::vector<Client>             _clients;
        std::map<std::string, Channel>  _channels;
//        std::vector<Channel>            _channels;
        std::string                     _serverName;
        commandMap_t                    _commandMap;

//Parsing and commands
        bool		parseClientInput(int fd, std::string buffer);

        bool		cap		(std::string &line, std::vector<Client>::iterator c);
        bool		invite	(std::string &line, std::vector<Client>::iterator c);
        bool		join	(std::string &line, std::vector<Client>::iterator c);
        bool		kick	(std::string &line, std::vector<Client>::iterator c);
        bool		mode	(std::string &line, std::vector<Client>::iterator c);
        bool		names	(std::string &line, std::vector<Client>::iterator c);
        bool		nick	(std::string &line, std::vector<Client>::iterator c);
        bool		notice	(std::string &line, std::vector<Client>::iterator c);
        bool		part	(std::string &line, std::vector<Client>::iterator c);
        bool		pass	(std::string &line, std::vector<Client>::iterator c);
        bool		ping	(std::string &line, std::vector<Client>::iterator c);
        bool		pong	(std::string &line, std::vector<Client>::iterator c);
        bool		privmsg	(std::string &line, std::vector<Client>::iterator c);
        bool		topic	(std::string &line, std::vector<Client>::iterator c);
        bool		user	(std::string &line, std::vector<Client>::iterator c);
        bool		quit	(std::string &line, std::vector<Client>::iterator c);

        void		join_channel(std::string &channelName, std::vector<Client>::iterator c);
        int         getIndexofClient(int fd);
        bool		channelExists(std::string nick);

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
        int&		getClientFd(std::vector<Client>::iterator c, int fd);
        std::vector<Client>::iterator getClientQUIET(int fd);
        void        start();

        //TODO: refactor to C++ camelCase instead of C under_scores
        void        ft_socket();
        void        accept_client();
        // bool        recv_client(int index);
        void        recv_client(int client_fd);
        void        quit_client(int client_fd);
        void        handle_send(int client_fd);
        bool        hasClient(int fd);

        std::vector<Client>& getClients() {
		return _clients;
	}
};
