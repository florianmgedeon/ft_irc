#include "../inc/Channel.hpp"

Channel::Channel() {
	_topicTimestamp = 0;
	//_userLimit = 0;
	//_hasPassword = false;
	//_inviteOnly = false;
	//_topicRestricted = false;
	_members.empty();
	_operators.empty();
	
}

Channel::~Channel() {}

Channel::Channel(std::string c) {
	_userLimit = 0;
	_hasPassword = false;
	_inviteOnly = false;
	_topicRestricted = false;
	Channel();
	_members.push_back(c);
	addOperator(c);
}

Channel::Channel(std::string c, std::string pwd): _password(pwd) {
	_userLimit = 0;
	_hasPassword = true;
	_inviteOnly = false;
	_topicRestricted = false;
	Channel();
	_members.push_back(c);
	addOperator(c);
}

//----------------------MEMBERS----------------------------

bool isNumber(std::string& str){

	std::istringstream str2(str);
	int i;
	str2 >> std::noskipws >> i;
	return str2.eof() && !str2.fail();
}

bool Channel::checkInvites(std::string nick){

	if (!hasInviteOnly())
		return true;
	if (std::find(_invites.begin(), _invites.end(), nick) == _invites.end())
		return false;
	return true;
}

bool Channel::checkUserLimit(){return (static_cast<long unsigned int>(getUserLimit()) < _members.size());}

bool Channel::executeMode(std::vector<std::string> tokens,
	std::vector<Client>::iterator c, std::vector<Client> &clients){
	//(void)c;
	//(void)clients;
	std::string argument = "";
	if (tokens.size() > 2)
		argument = tokens[2];
	if (tokens[1] == "+i"){
		setInviteOnly(true);
	}
	else if (tokens[1] == "-i"){
		setInviteOnly(false);
	}
	else if (tokens[1] == "+t"){
		setTopicRestricted(true);
	}
	else if (tokens[1] == "-t"){
		setTopicRestricted(false);
	}
	else if (tokens[1] == "-l"){
		setUserLimit(0);
	}
	else if (tokens[1] == "-k"){
		setPassword("");
		setHasPassword(false);
	}
	else if (tokens.size() < 3)
		return false;
	else if (tokens[1] == "+k"){
		setPassword(tokens[2]);
		setHasPassword(true);
	}
	else if (tokens[1] == "+o"){
		if (isMember(tokens[2]) && !isOperator(tokens[2]))
			addOperator(tokens[2]);
		else
			return false;
	}
	else if (tokens[1] == "-o"){
		if (isOperator(tokens[2]))
			removeOperator(tokens[2]);
		else
			return false;
	}
	else if (tokens[1] == "+l"){
		if (isNumber(tokens[2]) && atoi(tokens[2].c_str()) > 0 &&
			atoi(tokens[2].c_str()) == 	
			static_cast<int>(atoll(tokens[2].c_str())))
			setUserLimit(atoi(tokens[2].c_str()));
		else
			return false;
	}
	else
		return false;
	sendChannelMessage(c->getNickUserHost(), c->getNickUserHost() + " MODE #" +
		tokens[0] + " " + tokens[1] + " " + argument, clients);
	return true;
}

int Channel::getUserLimit(){
	return _userLimit;
}

void Channel::setInviteOnly(bool inviteOnly){
	_inviteOnly = inviteOnly;
}

void Channel::setPassword(std::string password){
	_password = password;
}

void Channel::setUserLimit(int userLimit){
	_userLimit = userLimit;
}

void Channel::setTopicRestricted (bool topicRestricted){
	_topicRestricted = topicRestricted;
}

void Channel::addInvites(std::string invites){
	_invites.push_back(invites);
}

void Channel::setHasPassword(bool hasPassword){

	_hasPassword = hasPassword;
}

bool Channel::hasInviteOnly(){
	return _inviteOnly;
}

bool Channel::getTopicRestricted(){return _topicRestricted;}

bool Channel::addMember(std::string c) {
	if (!isMember(c))
		return (_members.push_back(c), true);
	return false;
}

bool Channel::isMember(std::string nick) {
	return std::find(_members.begin(), _members.end(), nick) == _members.end() ? false : true;
}
void Channel::renameMember(std::string oldNick, std::string nick, std::vector<Client> &clients) {
	std::string msg =":" + oldNick + " NICK :" + nick;
	sendChannelMessage(oldNick, msg, clients);
	*std::find(_members.begin(), _members.end(), oldNick) = nick;
	if (isOperator(oldNick))
		*std::find(_operators.begin(), _operators.end(), oldNick) = nick;
}

void Channel::removeMember(std::string nick) {
	_members.erase(std::find(_members.begin(), _members.end(), nick));
	if (isOperator(nick))
		removeOperator(nick);
}

//--------------------OPERATORS--------------------------

void Channel::addOperator(std::string c) {
	if (isMember(c) && !isOperator(c)) {
		_operators.push_back(c);
	}
}

bool Channel::isOperator(std::string nick) {
	return std::find(_operators.begin(), _operators.end(), nick) == _operators.end() ? false : true;
}

void Channel::removeOperator(std::string nick) {
	_operators.erase(std::find(_operators.begin(), _operators.end(), nick));
	if (_operators.empty() && !_members.empty())
		addOperator(*_members.begin());
}

//------------------------------------------------

bool Channel::hasInvite(std::string nick){
	return std::find(_invites.begin(), _invites.end(), nick) != _invites.end();
}

bool Channel::isEmpty() {return _members.empty();}

void Channel::sendChannelMessage(std::string sender, std::string message, std::vector<Client> &clients) {
//std::cout <<"sender: " <<sender <<"\n";
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
