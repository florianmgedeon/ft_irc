#include "../inc/Channel.hpp"

Channel::Channel() {
	_topicTimestamp = 0;
	_hasPassword = false;
	_members.empty();
	_operators.empty();
}

Channel::~Channel() {}

Channel::Channel(std::string c) {
	Channel();
	_members.push_back(c);
	addOperator(c);
}

Channel::Channel(std::string c, std::string pwd): _password(pwd) {
	Channel();
	_members.push_back(c);
	addOperator(c);
	_hasPassword = true;
}

//----------------------MEMBERS----------------------------

bool Channel::addMember(std::string c) {
	if (!isMember(c))
		return (_members.push_back(c), true);
	return false;
}

bool Channel::isMember(std::string nick) {
	for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); it++)
		if (*it == nick)
			return true;
	return false;
}

void Channel::removeMember(std::string nick) {
	for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); it++)
		if (*it == nick)
			_members.erase(it);
	if (isOperator(nick))
		removeOperator(nick);
}

//--------------------OPERATORS--------------------------

void Channel::addOperator(std::string c) {
	if (isMember(c) && !isOperator(c)) {
		_operators.push_back(c);
//		std::cout << "added " << c->getNickname() << " to ops" << std::endl;
	}
}

bool Channel::isOperator(std::string nick) {
	for (std::vector<std::string>::iterator it = _operators.begin(); it != _operators.end(); it++)
		if (*it == nick)
			return true;
	return false;
}

void Channel::removeOperator(std::string nick) {
	for (std::vector<std::string>::iterator it = _operators.begin(); it != _operators.end(); it++)
		if (*it == nick)
			_operators.erase(it);
}

//------------------------------------------------

bool Channel::isEmpty() {return _members.empty();}

void Channel::sendChannelMessage(std::string sender, std::string message, std::vector<Client> &clients) {
//	std::cout << "channel message: " << message << " being sent to " << _members.size() << " clients." << std::endl;
	for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); ++it)
		if (*it != sender)
			for (std::vector<Client>::iterator itt = clients.begin(); itt != clients.end(); itt++)
				if (itt->getNickname() == *it)
					(*itt).sendToClient(message);
}


bool	Channel::checkPassword(std::string in) {return in == _password ? true : false;}

std::string	Channel::memberlist() {
	std::string res;
	for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); it++) {
		if (isOperator(*it)) res += "@";
		res = res + *it + " ";
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
