#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include <list>

class Client;

class Channel
{
    private:
        typedef void(Channel::*cmd_t)(std::string&);
        typedef std::map<std::string, cmd_t> commandMap_t;
        typedef commandMap_t::iterator  commandIter;
        std::string         _topic, _topicSetter;
        time_t				_topicTimestamp;
        std::string         _password;
        int                 _userLimit;
        bool                _inviteOnly;
        bool                _topicRestricted;
        bool                _hasPassword;
        std::vector<std::string> _invites;
        std::vector<std::string> _members;
        std::vector<std::string> _operators;
	commandMap_t                    _commandMap;
        void		iPlus	(std::string &line);
        void		iMinus	(std::string &line);
        void		tPlus	(std::string &line);
        void		tMinus	(std::string &line);
        void		kPlus	(std::string &line);
        void		kMinus	(std::string &line);
        void		oPlus	(std::string &line);
        void		oMinus	(std::string &line);
        void		lPlus	(std::string &line);
        void		lMinus	(std::string &line);

    public:
        Channel();
        ~Channel();
        Channel(std::string c);
        Channel(std::string c, std::string pwd);

	bool executeMode(std::string modestring, std::string argument);
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
		bool hasInvite(std::string nick);
		bool hasInviteOnly();
		std::string getTopicTimestamp() const;
		const std::string& getTopicSetter() const;
};
