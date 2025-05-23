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
        bool        _capNegotiation;
        bool        _isPasswordValid;
        bool        _isNickValid;
        bool        _isUSERcomplete;
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
        void        setUsername(std::string username);
        void        setHostname(std::string hostname);
        void        setServername(std::string servername);
        void        setRealname(std::string realname);
        void        setIsPasswordValid(bool isPasswordValid);
        void        setIsRegistered(bool isRegistered);
        bool        getIsPasswordValid() const;
        bool        getIsRegistered() const;
        void        setCapNegotiation(bool capNegotiation);
        bool        getCapNegotiation() const;
        void        setIsNickValid(bool isNickValid);
        bool        getIsNickValid() const;
        void        setIsUSERcomplete(bool isUSERcomplete);
        bool        getIsUSERcomplete() const;
        std::string getUsername() const;
        std::string getHostname() const;
        std::string getServername() const;
    };

#endif
