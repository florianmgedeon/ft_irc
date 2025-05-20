#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"


class Client
{
    private:
        std::string _nickname;
        std::string _username;
        std::string _realname;
        std::string _hostname;
        std::string _servername;
        bool        _write_ready;
        bool        _isPasswordValid;
        bool        _isRegistered;
        pollfd      *pfd;

    public:
        std::string send_buffer;
        std::string recv_buffer;
        Client(std::string hostname, pollfd *pfd);
        Client();
        ~Client();
        void        setWrite(bool write);
        int         getFd() const;
        void        append_send_buffer(std::string message);
        void        append_recv_buffer(char *buffer);
        void        setNickname(std::string nickname);
        std::string &getNickname(void);
        bool        getIsRegistered() const;
        void        setUsername(std::string username);
        void        setHostname(std::string hostname);
        void        setServername(std::string servername);
        void        setRealname(std::string realname);
        void        setIsRegistered(bool isRegistered);
    };

#endif
