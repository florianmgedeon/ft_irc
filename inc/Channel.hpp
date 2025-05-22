#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"
#include "Client.hpp"
#include <list>

class Client;

class Channel
{
    private:
        std::string         _name;
        std::string         _topic;                  
        std::string         _password;                
        //int                 _userLimit;              
        //bool                _inviteOnly;
        //bool                _topicRestricted;
        //bool                _hasPassword;
        std::list<Client *> _members; // could be a map<nickname, Client *> to be easier to search?
        std::list<Client *> _operators;
        std::list<Client *> _banlist;

    public:
        Channel();
        ~Channel();
        bool isMember(Client *client);
        bool isOperator(Client *client);
        void removeMember(Client *client);
        bool isEmpty();
        void addOperator(Client *client);
        void removeOperator(Client *client);
        void sendChannelMessage(std::string sender, std::string message);
};

#endif
