#include "../inc/Channel.hpp"

Channel::Channel(){_hasPassword = false;}

Channel::~Channel(){}

Channel::Channel(std::string pwd): _password(pwd) {_hasPassword = true;}

//----------------------MEMBERS----------------------------

bool Channel::addMember(Client *client) {
	if (_members.find(client->getNickname()) == _members.end() &&
		_banlist.find(client->getNickname()) == _banlist.end()) {
		_members.insert(std::make_pair(client->getNickname(), client));
		return true;
	}
	return false;
}

bool Channel::isMember(std::string &nick) {return _members.find(nick) != _members.end();}

void Channel::removeMember(Client *client) {
	_members.erase(client->getNickname());
	if (isOperator(client->getNickname()))
		removeOperator(client);
}

//--------------------OPERATORS--------------------------

void Channel::addOperator(Client *client) {
	if (isMember(client->getNickname()) && !isOperator(client->getNickname()))
		_operators.insert(std::make_pair(client->getNickname(), client));
}

bool Channel::isOperator(std::string &nick) {return _operators.find(nick) != _operators.end();}

void Channel::removeOperator(Client *client) {_operators.erase(client->getNickname());}

//------------------------------------------------

bool Channel::isEmpty() {return _members.empty();}

void Channel::sendChannelMessage(std::string sender, std::string message) {
	if (!isMemberBanned(sender))
	for (std::map<std::string, Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
		(*it).second->sendToClient(message);
}

bool	Channel::isMemberBanned(std::string &nick) {return _banlist.find(nick) != _banlist.end();}

bool	Channel::checkPassword(std::string in) {return in == _password ? true : false;}
