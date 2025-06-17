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
        int                 _userLimit;
        bool                _inviteOnly;
        bool                _topicRestricted;
        //bool                _hasPassword;
        std::vector<std::string> _invites;
        std::vector<std::string> _members;
        std::vector<std::string> _operators;

    public:
        Channel();
        ~Channel();
        Channel(std::string c, std::string name);
        Channel(std::string c, std::string name, std::string pwd);

	void setInviteOnly(bool inviteOnly);
	void setPassword(std::string password);
	void setUserLimit(int userLimit);
	void setTopicRestricted (bool topicRestricted);
	void addInvites(std::string invites);
	//void setHasPassword(bool hasPassword);
	
	bool addInvite(std::string nick);
	int getUserLimit();
	bool getTopicRestricted();
	bool checkUserLimit();
	bool checkInvites(std::string nick);
	bool executeMode(std::vector<std::string> tokens, std::vector<Client>::iterator c, std::vector<Client> &clients);
	bool addMember(std::string c);
	bool isMember(std::string nick);
	void renameMember(std::string oldNick, std::string nick, std::vector<Client> &clients);
	void removeMember(std::string nick, std::vector<Client> &clients);

	void addOperator(std::string c);
	bool isOperator(std::string nick);
	void removeOperator(std::string nick, std::vector<Client> &clients);

	bool isEmpty();
	void sendChannelMessage(std::string sender, std::string message, std::vector<Client> &clients);
	bool checkPassword(std::string in);

	const std::string& getTopic() const;
	std::string	memberlist();
	void setTopic(const std::string &topic, const std::string &setter);
	bool hasTopic();
	bool hasInvite(std::string nick);
	bool hasInviteOnly();
	std::string getTopicTimestamp() const;
	const std::string& getTopicSetter() const;
};
