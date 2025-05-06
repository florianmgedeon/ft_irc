#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <iostream>
#include <string>

class Server
{
    private:
        int         _port;
        std::string _password;
    public:
        Server(int port, std::string password);
        ~Server();
        int         getPort() const;
        std::string getPassword() const;

};

#endif