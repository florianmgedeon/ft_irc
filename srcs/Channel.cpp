#include "../inc/Channel.hpp"

Channel::Channel() {
	_hasPassword = false;
	_topicTimestamp = 0;
}

Channel::~Channel(){}

Channel::Channel(std::string pwd): _password(pwd) {
	_hasPassword = true;
	_topicTimestamp = 0;
}

//----------------------MEMBERS----------------------------

bool Channel::addMember(Client *client, bool makeOp) {
	if (!isMember(client->getNickname()) && !isMemberBanned(client->getNickname())) {
		_members.insert(std::make_pair(client->getNickname(), client));
		if (makeOp)
			addOperator(client);
		return true;
	}
	return false;
}

bool Channel::isMember(std::string nick) {return _members.find(nick) != _members.end();}

void Channel::removeMember(std::string nick) {
	_members.erase(nick);
	if (isOperator(nick))
		removeOperator(nick);
}

//--------------------OPERATORS--------------------------

void Channel::addOperator(Client *client) {
	if (isMember(client->getNickname()) && !isOperator(client->getNickname())) {
		_operators.insert(std::make_pair(client->getNickname(), client));
//		std::cout << "added " << client->getNickname() << " to ops" << std::endl;
	}
}

bool Channel::isOperator(std::string nick) {return _operators.find(nick) != _operators.end();}

void Channel::removeOperator(std::string nick) {_operators.erase(nick);}

//------------------------------------------------

bool Channel::isEmpty() {return _members.empty();}

void Channel::sendChannelMessage(std::string sender, std::string message) {
//	std::cout << "channel message: " << message << std::endl;
	if (!isMemberBanned(sender))
		for (std::map<std::string, Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
			if (it->first != sender)
				it->second->sendToClient(message);
}

bool	Channel::isMemberBanned(std::string &nick) {return _banlist.find(nick) != _banlist.end();}

bool	Channel::checkPassword(std::string in) {return in == _password ? true : false;}

std::string	Channel::memberlist() {
	std::string res;
	for (std::map<std::string, Client *>::iterator it = _members.begin(); it != _members.end(); it++) {
		if (isOperator(it->first)) res += "@";
		res = res + it->first + " ";
	}
	return res;
}

const std::string&	Channel::getTopic() const {return _topic;}

void	Channel::setTopic(const std::string &topic, const std::string &setter) {
	_topic = topic;
	_topicTimestamp = std::time(0);
	_topicSetter = setter;
}

bool	Channel::hasTopic() {return _topic.size() == 0 ? false : true;}

std::string Channel::getTopicTimestamp() const {
	std::ostringstream res;
	res << _topicTimestamp;
	return res.str();
}

const std::string& Channel::getTopicSetter() const {
	return _topicSetter;
}
