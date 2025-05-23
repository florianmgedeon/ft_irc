#include "../inc/Channel.hpp"

Channel::Channel(){
	_hasPassword = false;
}

Channel::~Channel(){}

Channel::Channel(std::string pwd): _password(pwd) {
	_hasPassword = true;
}

bool Channel::isMember(Client *client)
{
    for (std::list<Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}

void Channel::removeMember(Client *client)
{
    for (std::list<Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        if (*it == client)
        {
            _members.erase(it);
            if (isOperator(client))
                removeOperator(client);
            break;
        }
    }
}

bool Channel::isEmpty()
{
    return _members.empty();
}

bool Channel::isOperator(Client *client)
{
    for (std::list<Client *>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}

void Channel::addOperator(Client *client)
{
    if (isMember(client) && !isOperator(client))
        _operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
    for (std::list<Client *>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == client)
        {
            _operators.erase(it);
            break;
        }
    }
}

void Channel::sendChannelMessage(std::string sender, std::string message) {
	for (std::list<Client *>::iterator it = _banlist.begin(); it != _banlist.end(); it++)
		if ((*it)->getNickname() == sender)
			return;
	for (std::list<Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
		(*it)->sendToClient(message);
}

bool	Channel::isMemberBanned(Client *client) {
    for (std::list<Client *>::iterator it = _banlist.begin(); it != _banlist.end(); ++it)
        if (*it == client)
            return true;
    return false;
}

bool	Channel::checkPassword(std::string in) {
	if (in == _password) return true;
	return false;
}
