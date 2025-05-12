#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"


class Client
{
    private:
        std::string _nickname;
        std::string _clientname;
        std::string _hostname;
        std::string _serverName;
        int         _fd;
        bool        write_ready;

    public:
        std::string send_buffer;
        void        setWrite(bool write);
        int        getFd() const;
        Client();
        Client(std::string hostname, int fd);
        ~Client();
    };

#endif