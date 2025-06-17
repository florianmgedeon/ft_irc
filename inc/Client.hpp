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
        bool        _capNegotiation;
        bool        _isPasswordValid;
        bool        _isRegistered;
        bool        _isUserComplete;
        bool        _isNickValid;
        int         _epollfd;
        struct epoll_event _ev;

    public:
        std::string send_buffer;
        std::string recv_buffer;
        std::string _parsable;
        Client(std::string hostname, struct epoll_event _ev, int epollfd);
        Client();
        ~Client();
        int         getFd() const;
        void        sendToClient(std::string message);
        void        recvFromClient(char *buffer);
        void        setNickname(std::string nickname);
        std::string &getNickname(void);
        std::string getColNick(void);
        std::string getNickUserHost(void);
        std::string getColHost(void);
        bool        getIsRegistered() const;
        void        setIsRegistered(bool isRegistered);
        void        setUsername(std::string username);
        void        setHostname(std::string hostname);
        void        setServername(std::string servername);
        void        setRealname(std::string realname);
        void        setCapNegotiation(bool capNegotiation);
        bool		isIsPasswordValid() const;
        void		setIsPasswordValid(bool isPasswordValid);
        bool        getIsPasswordValid() const;
        bool        getIsUserComplete() const;
        void        setIsUserComplete(bool isUserComplete);
        bool        getIsNickValid() const;
        void        setIsNickValid(bool isNickValid);
        bool        getCapNegotiation() const;
        std::string getHostname() const;
        std::string getUsername() const;
        std::string getServername() const;

        struct epoll_event &getEv();

};
