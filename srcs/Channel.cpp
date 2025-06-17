#include "../inc/Channel.hpp"

Channel::Channel() {
	_topicTimestamp = 0;
	_userLimit = 0;
	_inviteOnly = false;
	_topicRestricted = false;
	_members.empty();
	_operators.empty();
	
}

Channel::~Channel() {}

Channel::Channel(std::string c, std::string name): _name(name) {
	_userLimit = 0;
	_inviteOnly = false;
	_topicRestricted = false;
	Channel();
	_members.push_back(c);
	addOperator(c);
}

Channel::Channel(std::string c, std::string name, std::string pwd): _name(name), _password(pwd) {
	_userLimit = 0;
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

void Channel::lastOp(std::vector<Client>::iterator c, std::vector<Client> &clients){

	std::string client = _members[0];
	if (_members.size() > 1 && isOperator(c->getNickname())
		&& _operators.size() == 1){
		if (_members[0] == c->getNickname())
			client = _members[1];
		sendChannelMessage(c->getNickname(), c->getNickUserHost() + " MODE #" +
			_name + " +o " + client, clients);
	}
}

bool Channel::checkInvites(std::string nick){

	if (!hasInviteOnly())
		return true;
	if (std::find(_invites.begin(), _invites.end(), nick) == _invites.end())
		return false;
	return true;
}

bool Channel::checkUserLimit(){

if (!getUserLimit())
	return true;
return (static_cast<long unsigned int>(getUserLimit()) > _members.size());
}

bool Channel::executeMode(std::vector<std::string> tokens,
	std::vector<Client>::iterator c, std::vector<Client> &clients){
	//(void)c;
	//(void)clients;
	std::string argument = "";
	if (tokens.size() > 2)
		argument = tokens[2];
	if (tokens[1].length() != 2 || (tokens[1].substr(0, 1) != "+" &&
		tokens[1].substr(0, 1) != "-" ) )
		return false;
	
	char prefix = tokens[1].at(0);
	char mode = tokens[1].at(1);
	
	if ( (mode == 'i' || mode == 't' ||
		((mode == 'l') && prefix == '-')) && argument.length())
		return false;
	
	if ((((mode == 'l') && prefix == '+') || mode == 'o' || mode == 'k')
		&& !argument.length())
		return false;
	switch(mode){
	
		case 'i':{
			prefix == '+' ? setInviteOnly(true) : setInviteOnly(false);
			break;
		}
		case 't':{
			prefix == '+' ? setTopicRestricted(true) :
				setTopicRestricted(false);
			break;
		}
		case 'k':{
			prefix == '+' ? (setPassword(argument)) : setPassword("");
			break;
		}
		case 'o':{
		
			if (prefix == '+' && isMember(argument) &&
				!isOperator(argument))
				addOperator(argument);
			else if (prefix == '-' && isOperator(argument) &&
				argument != c->getNickname()) {
				removeOperator(argument);
			}
			else
				return false;
			break;
		}
		case 'l':{
			if (prefix == '+' && isNumber(argument)
				&& atoi(argument.c_str()) > 0 &&
				atoi(argument.c_str()) ==
				static_cast<int>(atoll(argument.c_str())))
				setUserLimit(atoi(argument.c_str()));
			else if (prefix == '-')
				setUserLimit(0);
			else
				return false;
			break;
		}
	}
	sendChannelMessage("", c->getNickUserHost() + " MODE #" +
		tokens[0] + " " + tokens[1] + " " + argument, clients);
	return true;
}

bool Channel::addInvite(std::string nick){
	
	if (std::find(_invites.begin(), _invites.end(), nick) == _invites.end()){	
		_invites.push_back(nick);
		return true;
	}
	return false;
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
/*
void Channel::setHasPassword(bool hasPassword){

	_hasPassword = hasPassword;
}
*/
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


bool	Channel::checkPassword(std::string in) {return in == _password || !_password.length();}

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
