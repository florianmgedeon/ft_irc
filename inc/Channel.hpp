#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include <list>

class Client;

class Channel
{
    private:
		std::string 		_name;
        std::string         _topic, _topicSetter;
        time_t				_topicTimestamp;
        std::string         _password;
        //int                 _userLimit;
        //bool                _inviteOnly;
        //bool                _topicRestricted;
        bool                _hasPassword;
        std::vector<std::string> _members;
        std::vector<std::string> _operators;

    public:
        Channel();
        ~Channel();
        Channel(std::string c);
        Channel(std::string c, std::string pwd);

        bool addMember(std::string c);
        bool isMember(std::string nick);
        void removeMember(std::string nick);

        void addOperator(std::string c);
        bool isOperator(std::string nick);
        void removeOperator(std::string nick);

        bool isEmpty();
        void sendChannelMessage(std::string sender, std::string message, std::vector<Client> &clients);
        bool checkPassword(std::string in);

        const std::string& getTopic() const;
		std::string	memberlist();
		void setTopic(const std::string &topic, const std::string &setter);
		bool hasTopic();
		std::string getTopicTimestamp() const;
		const std::string& getTopicSetter() const;
};
