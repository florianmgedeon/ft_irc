#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include <list>

class Client;

class Channel
{
    private:
//        std::string         _name;
        std::string         _topic;                  
        std::string         _password;                
        //int                 _userLimit;              
        //bool                _inviteOnly;
        //bool                _topicRestricted;
        bool                _hasPassword;
        std::map<std::string, Client *> _members;
        std::map<std::string, Client *> _operators;
        std::map<std::string, Client *> _banlist;//TODO: research how bans are handled and saved überhaupt

    public:
        Channel();
        ~Channel();
        Channel(std::string pwd);

        bool addMember(Client *client, bool makeOp);
        bool isMember(std::string nick);
        void removeMember(Client *client);

        void addOperator(Client *client);
        bool isOperator(std::string nick);
        void removeOperator(Client *client);

        bool isEmpty();
        void sendChannelMessage(std::string sender, std::string message);
        bool isMemberBanned(std::string &nick);
        bool checkPassword(std::string in);

        const std::string& getTopic() const;
		void setTopic(const std::string &topic);
		bool hasTopic();
		std::string	memberlist();
};
