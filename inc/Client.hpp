#pragma once

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
        bool        _capNegotiation;
        bool        _isPasswordValid;
        bool        _isRegistered;
        pollfd      *_pfd;

    public:
        std::string send_buffer;
        std::string recv_buffer;
        Client(std::string hostname, pollfd *pfd);
        Client();
        ~Client();
        void        setWrite(bool write);
        int         getFd() const;
        void        sendToClient(std::string message);
        void        recvFromClient(char *buffer);
        void        setNickname(std::string nickname);
        std::string &getNickname(void);
        std::string getColNick(void);
        std::string getNickUserHost(void);
        bool        getIsRegistered() const;
        void        setIsRegistered(bool isRegistered);
        void        setUsername(std::string username);
        void        setHostname(std::string hostname);
        void        setServername(std::string servername);
        void        setRealname(std::string realname);
        void        setCapNegotiation(bool capNegotiation);
        bool		isIsPasswordValid() const;
        void		setIsPasswordValid(bool isPasswordValid);
};
