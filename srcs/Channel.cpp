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
	_commandMap.insert(std::make_pair("+i",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-i",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+t",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-t",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+k",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-k",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+o",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-o",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+l",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-l",	&Channel::iMinus));
	Channel();
	_members.push_back(c);
	addOperator(c);
}

Channel::Channel(std::string c, std::string pwd): _password(pwd) {
	_userLimit = 0;
	_hasPassword = true;
	_inviteOnly = false;
	_topicRestricted = false;
	_commandMap.insert(std::make_pair("+i",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-i",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+t",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-t",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+k",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-k",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+o",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-o",	&Channel::iMinus));
	_commandMap.insert(std::make_pair("+l",	&Channel::iPlus));
	_commandMap.insert(std::make_pair("-l",	&Channel::iMinus));
	Channel();
	_members.push_back(c);
	addOperator(c);
}

//----------------------MEMBERS----------------------------

bool Channel::executeMode(std::string modestring, std::string argument){
	
	commandIter comMapIt = _commandMap.find(modestring);	
			if (comMapIt != _commandMap.end()){
				(this->*(comMapIt->second))(argument);
				return true;
	}
	return false;
}

//TODO: check for requirements (must or must not have argument)
void Channel::iPlus(std::string& argument){
	(void)argument;
	
	_inviteOnly = true;
}

void Channel::iMinus(std::string& argument){
	(void)argument;
	
	_inviteOnly = false;
}

void Channel::tPlus(std::string& argument){
	(void)argument;
	
	_topicRestricted = true;
}

void Channel::tMinus(std::string& argument){
	(void)argument;
	
	_topicRestricted = false;
}

void Channel::kPlus(std::string& argument){

	_hasPassword = true;
	_password = argument;
}

void Channel::kMinus(std::string& argument){
	(void)argument;
	
	//TODO: must have an argument, vermutl compare mit current pw
	_hasPassword = false;
}

void Channel::oPlus(std::string& argument){

	if (isMember(argument) && !isOperator(argument))
		addOperator(argument);
}

void Channel::oMinus(std::string& argument){
	//TODO: verhindern, dass founder removed wird, sich selbst removen?
	//evtl in commands handeln
	if (isOperator(argument))
		removeOperator(argument);
}

void Channel::lPlus(std::string& argument){
	
	if (atoi(argument.c_str()) > 0 && atoi(argument.c_str()) ==
		static_cast<int>(atoll(argument.c_str())))
		_userLimit = atoi(argument.c_str());
}

void Channel::lMinus(std::string& argument){
	(void)argument;
	//TODO: must not have an argument
	_userLimit = 0;
}

bool Channel::hasInviteOnly(){
	return _inviteOnly;
}

bool Channel::addMember(std::string c) {
	if (!isMember(c))
		return (_members.push_back(c), true);
	return false;
}

bool Channel::isMember(std::string nick) {
	return std::find(_members.begin(), _members.end(), nick) == _members.end() ? false : true;
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
}

//------------------------------------------------

bool Channel::hasInvite(std::string nick){
	return std::find(_invites.begin(), _invites.end(), nick) != _invites.end();
}

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
