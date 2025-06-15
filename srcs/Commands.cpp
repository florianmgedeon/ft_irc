#include "../inc/Server.hpp"
typedef std::map<std::string, bool(Server::*)(std::string&, std::vector<Client>::iterator)>::iterator commandIter;
typedef std::map<std::string, Channel>::iterator channelIter;

std::string	tokenize(std::string &line, char c) {
	std::string res; res.clear();
//	if (line.find(c) != std::string::npos) { funktioniert aus irgndam grund net... logon hängt
	if (line.size() && line.find(c)) {
		res = line.substr(0, line.find(c));
		line = line.substr(line.find(c) + 1);
	}
	return res;
}

std::string	strPastColon(std::string &line) {
	std::string res; res.clear();
	if (line.size() && line.find(':') != std::string::npos)
		res = line.substr(line.find(':') + 1);
	return res;
}

void	stripPrefix(std::string &line){

if (!line.length())
	return;
line = line.substr(1);

}

bool	Server::parseClientInput(int fd, std::string buffer) {
	std::string line, dummy;
	std::stringstream streamline;
	streamline << buffer;
//	bool res = true;
	while (std::getline(streamline, line, '\r')) {
		std::getline(streamline, dummy, '\n');
		if (!dummy.size()) {
			std::string cmd = tokenize(line, ' ');
			transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
			commandIter comMapIt = _commandMap.find(cmd);	//find upcased command string in command map
			if (comMapIt != _commandMap.end())
			{
				if (getClientQUIET(fd) == _clients.end())
					return (false);
				/*res = */(this->*(comMapIt->second))(line, getClient(fd));	//execute command
			}
			else getClient(fd)->sendToClient(getClient(fd)->getColNick() + " " + cmd + " :Unknown command");
		}
	}
	return true;
}


//---------------------------COMMANDS------------------------------------------

bool	Server::cap(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "CAP line: <" << line << ">" << std::endl;
	std::string param = tokenize(line, ' ');
	if (param == "LS")
		return (c->sendToClient(":" + _serverName + " CAP * LS :"), true);
	if (param == "REQ")
		return (c->sendToClient(":" + _serverName + " CAP * NAK :" + line), true);
	if (param == "END")
	{
		c->setCapNegotiation(true);
		if (c->getIsPasswordValid() && c->getIsUserComplete() && c->getIsNickValid() && !c->getIsRegistered())
		{
			c->setIsRegistered(true);
			c->sendToClient(":" + _serverName + " 001 " + c->getNickname() + " :Welcome to the Internet Relay Network " + c->getNickname() + "!" + c->getUsername() + "@" + c->getHostname());
		}
	}
	return (true);
}
//TODO: invite
bool	Server::invite(std::string &line, std::vector<Client>::iterator c) {
	(void)line; (void)c;
	return true;
}

void	Server::join_channel(std::string &channelName, std::string &channelPassword,
	std::vector<Client>::iterator c) {
	_channels[channelName].addMember(c->getNickname());
	_channels[channelName].sendChannelMessage(c->getNickUserHost(), c->getNickUserHost() + " JOIN #" + channelName + " " + channelPassword, getClients());
//std::cout <<"\n\nJOINING CHANNEL \n\n";	
	channelName = "#" + channelName;
	topic(channelName, c);
	names(channelName, c);
}

bool	Server::join(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "join inb4: |" << line << "|" << std::endl;

	if (!line.length() || line == ":")
		return false;

//std::cout <<"line" <<line <<"_________\n";
	std::string readName, readPwd;
	std::stringstream pwdstream, chanNamestream;
	chanNamestream << tokenize(line, ' ');
	pwdstream << line;
	
	while (std::getline(chanNamestream, readName, ',')) {
		stripPrefix(readName);
		if (_channels.find(readName) == _channels.end()) {	//create channel
			std::getline(pwdstream, readPwd, ',');
			if (readPwd.size()) {	//create pw-locked channel
				std::cout << "creating locked channel " << readName << " pwd " << readPwd << std::endl;
				_channels.insert(std::make_pair(readName, Channel(c->getNickname(), readPwd)));
				join_channel(readName, readPwd, c);
			} else {	//create open chanel
				std::cout << "creating open channel " << readName << std::endl;
				_channels.insert(std::make_pair(readName, Channel(c->getNickname())));
				join_channel(readName, readPwd, c);
			} //else return (c->sendToClient(":" + readName + " 476 :Bad Channel Mask"), false); //TODO: reimplement error

		} else {	//try joining existing channel
			std::getline(pwdstream, readPwd, ',');	//join pw-locked channel
			if (!(_channels[readName].checkPassword(readPwd)))
				return (c->sendToClient("475 " + c->getNickUserHost() 
					+ " #" + readName + 
					" :Cannot join channel (+k)"), false);
			if (!_channels[readName].checkInvites(c->getNickname()))
				return (c->sendToClient("473 " + c->getNickUserHost() 
					+ " #" + readName + 
					" :Cannot join channel (+i)"), false);
			if (!_channels[readName].checkUserLimit())
				return (c->sendToClient("471 " + c->getNickUserHost() 
					+ " #" + readName + 
					" :Cannot join channel (+l)"), false);	
			join_channel(readName, readPwd ,c);
		}
	}
	return true;
}

bool	Server::kick(std::string &line, std::vector<Client>::iterator c) {
	std::cout << "line: |" << line << "|" << std::endl;
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 KICK :Not enough parameters"), false);
	std::string channel = tokenize(line, ' ');
	stripPrefix(channel);
	if (!channelExists(channel))
		return (c->sendToClient(c->getColNick() + " 403 :No such channel"), false);
	std::string user = tokenize(line, ' ');
	if (!(_channels[channel].isMember(user)))
		return (c->sendToClient(c->getColNick() + " 441 " + user + " " + channel + " :They aren't on that channel"), false);
	if (!(_channels[channel].isMember(c->getNickname())))
		return (c->sendToClient(c->getColNick() + " 442 " + channel + " :You're not on that channel"), false);
	if (!(_channels[channel].isOperator(c->getNickname())))
		return (c->sendToClient(c->getColNick() + " 482 " + channel + " :You're not channel operator"), false);
	std::string reason = strPastColon(line);

	getClient(user)->sendToClient(c->getNickUserHost() + " KICK " + channel + " " + user + " :" + reason);
	_channels[channel].removeMember(user);
	_channels[channel].sendChannelMessage(c->getNickname(), c->getNickUserHost() + " KICK " + channel + " " + user, getClients());
	return true;
}

//TODO:mode------------modes: +- i,t,k,o,l
bool	Server::mode(std::string &line, std::vector<Client>::iterator c) {
	if (!line.size())
		return (c->sendToClient(c->getColNick() +
			" 461 MODE :Not enough parameters"), false);

	std::istringstream iss(line);
	std::string token;
	std::vector<std::string> tokens;
	while (iss >> token)
		tokens.push_back(token);
	
	if (tokens.size() < 2 || !tokens[0].length())
		return false;
	if (tokens[0] == c->getColNick().substr(1))
		return false;
	if (!channelExists(tokens[0]))
		return (c->sendToClient(c->getColNick() + 
			" 403 MODE :No such channel"), false);
	if (!(_channels[tokens[0]].isOperator(c->getNickname())))
		return (c->sendToClient(c->getColNick() + " 482 " + tokens[0]
			+ " :You're not channel operator"), false);

	return _channels[tokens[0]].executeMode(tokens, c, getClients());
}

bool	Server::names(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "line: |" << line << "|" << std::endl;
	std::stringstream linestream;
	std::string channelName, res;
	linestream << line;
	while (std::getline(linestream, channelName, ',')) {
		stripPrefix(channelName);
		res += c->getColHost() + " 353 " + c->getNickname() + " = #" + channelName + " :" + _channels[channelName].memberlist();
	}
	c->sendToClient(res);
	c->sendToClient(c->getColHost() + " 366 " + c->getNickname() + " #" + channelName + " :End of NAMES list");
	return true;
}

bool	Server::nick(std::string &line, std::vector<Client>::iterator c) {
	if (c->getIsPasswordValid() == false)
		return (false);
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 431 " + c->getNickname() + " :No nickname given"), false);

	//Syntax check
	const std::string specials = "[]\\`_^{|}";
	if ((!std::isalpha(line[0]) && specials.find(line[0]) == std::string::npos) || line.find_first_of(" \r\n") != std::string::npos)
		return (c->sendToClient(c->getColNick() + " 432 " + line + " :Erroneous nickname"), false);
    for (size_t j = 1; j < line.length(); ++j)
    {
        char x = line[j];
        if (!std::isalnum(x) && specials.find(x) == std::string::npos)
			return (c->sendToClient(c->getColNick() + " 432 " + line + " :Erroneous nickname"), false);
    }
    if (line.find_first_of("0123456789") != std::string::npos && line.find_first_not_of("0123456789") == std::string::npos)
		return (c->sendToClient(c->getColNick() + " 432 " + line + " :Erroneous nickname"), false);

	//Length check, if long - truncate
	if (line.length() > 30)
		line = line.substr(0, 30);
	//Availability check
	if (getClient(line) != _clients.end())
		return (c->sendToClient(c->getColNick() + " 433 " + c->getNickname() + " :Nickname is already in use"), false);

	//save previous nickname
	std::string oldNick = "*";
	bool nickSet = false;
	if (c->getIsRegistered())
		oldNick = c->getNickname();
	else {
		c->setNickname(line);
		c->setIsNickValid(true);
		nickSet = true;
	}
	//Register and/or announce NICK
	if (c->getCapNegotiation() && c->getIsUserComplete() && !c->getIsRegistered()) {
		c->setIsRegistered(true);
		c->sendToClient(":" + c->getServername() + " 001 " + c->getNickname() + " :Welcome to the Internet Relay Network " + c->getNickname() + "!" + c->getUsername() + "@" + c->getHostname());
		for (size_t i = 0; i < _clients.size(); i++)
			_clients[i].sendToClient(c->getColNick() + " NICK " + line);
	}
	if (nickSet)
		return true;

	//nickname change
	c->sendToClient(":" + oldNick + " NICK :" + line);
	for (channelIter it = _channels.begin(); it != _channels.end(); it++)
		if (it->second.isMember(oldNick))
			it->second.renameMember(oldNick, line, _clients);
	c->setNickname(line);

	return true;
}

bool	Server::notice(std::string &line, std::vector<Client>::iterator c) {
	(void)line; (void)c;
	return true;
}

bool	Server::part(std::string &line, std::vector<Client>::iterator c) {
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 PART :Not enough parameters"), false);
	std::stringstream linestream;
	std::string channelName, channels, reason;
	
	channels = tokenize(line, ' ');
	reason = strPastColon(line);
//	std::cout << "line: |" << line << "|" << std::endl;
	linestream << channels;
	
	while (std::getline(linestream, channelName, ',')) {
		stripPrefix(channelName);
		if (!channelExists(channelName))
			c->sendToClient(c->getColNick() + " 403 :No such channel");
		else if (!_channels[channelName].isMember(c->getNickname()))
			c->sendToClient(c->getColNick() + " 442 #" + channelName + " :You're not on that channel");
		else {
			_channels[channelName].sendChannelMessage(c->getNickname(), c->getNickUserHost() + " PART #" + channelName, getClients());
			c->sendToClient(c->getNickUserHost() + " PART #" + channelName + " :" + reason);
			_channels[channelName].removeMember(c->getNickname());
			if (_channels[channelName].isEmpty())
				_channels.erase(channelName);
		}
	}
	return true;
}

int Server::getIndexofClient(int fd) {
    for (size_t i = 0; i < _clients.size(); i++)
        if (_clients[i].getFd() == fd)
            return i;
    return -1;
}

bool	Server::pass(std::string &line, std::vector<Client>::iterator c) {
	// if (c->getCapNegotiation() == false)
		// return (c->sendToClient(c->getColNick() + " 421 PASS :CAP negotiation not finished"), false);
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 PASS :Not enough parameters"), false);
	if (c->getIsRegistered())
		return (c->sendToClient(c->getColNick() + " 462 :You may not reregister"), false);
	if (line == this->getPassword())
		return (c->setIsPasswordValid(true), true);
	else 
	{
		c->sendToClient(c->getColNick() + " 464 :Password incorrect");
		std::string x = "";
		std::cout << "pass-quit" << std::endl;
		quit(x, c);
		return (false);
	}
}

bool	Server::ping(std::string &line, std::vector<Client>::iterator c)//for when server gets ping from client
{
	if (line.empty())
		return (c->sendToClient(c->getColNick() + " 409 :No origin specified"), false);
	c->sendToClient(":" + _serverName + " PONG " + line);
	return (true);
}

bool	Server::pong(std::string &line, std::vector<Client>::iterator c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::privmsg(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "msg: |" << line << "|" << std::endl;
	std::stringstream namestream;
	std::string username, users, msg;
	users =  tokenize(line, ' ');
	if (!users.size())
		return (c->sendToClient(c->getColNick() + " 411 :No recipient given (PRIVMSG)"), false);
	msg = strPastColon(line);
	if (!msg.size())
		return (c->sendToClient(c->getColNick() + " 412 :No text to send"), false);

	namestream << users;
	while (std::getline(namestream, username, ',')) {
		bool toChannel = false;
		while (strchr("@%", username[0])) {
			if (username[0] == '@') username = username.substr(1); //TODO: send to channel ops
			else if (username[0] == '%') username = username.substr(1); //TODO: send to channel ops
		}
		if (username[0] == '#' || username[0] == '%')
				toChannel = true;
		if (toChannel) {
			stripPrefix(username);
			if (_channels.find(username) == _channels.end())
				return (c->sendToClient(c->getColNick() + " 401 :No such channel"), false);
			if (_channels[username].isMember(c->getNickname()))
				_channels[username].sendChannelMessage(c->getNickname(), c->getNickUserHost() + " PRIVMSG #" + username + " :" + msg, getClients());
		} else {
			std::vector<Client>::iterator recp = getClient(username);
			if (recp == _clients.end())
				return (c->sendToClient(c->getColNick() + " 401 :No such nick"), false);
//			std::cout << c->getNickname() << " sending privmsg to " << (*recp).getNickname() << ": " << msg << std::endl;
			(*recp).sendToClient(c->getNickUserHost() + " PRIVMSG " + (*recp).getNickname() + " :" + msg);
		}
	}
	return true;
}

bool	Server::topic(std::string &line, std::vector<Client>::iterator c) {
	std::cout << "topic: |" << line << "|" << std::endl;
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 TOPIC :Not enough parameters"), false);
	std::string channelName = tokenize(line, ' ');
	std::string newTopic = strPastColon(line);
	stripPrefix(channelName);
	std::cout << "channelName: " << channelName << ", newTopic: " << newTopic << std::endl;
	if (!channelExists(channelName))
		return (c->sendToClient(c->getColNick() + " 403 :No such channel"), false);
	if (newTopic.size()) {	//set new topic
		if (!_channels[channelName].isOperator(c->getNickname()) &&
			_channels[channelName].getTopicRestricted())
			return (c->sendToClient(c->getColNick() + " 482 #" + channelName + " :You're not channel operator"), false);
		line = strPastColon(line);
		_channels[channelName].setTopic(newTopic, c->getNickname());
		_channels[channelName].sendChannelMessage("", c->getColHost() + " 332 " + c->getNickname() + " #" + channelName + " :" + _channels[channelName].getTopic(), getClients());
		_channels[channelName].sendChannelMessage("", c->getColHost() + " 333 " + c->getNickname() + " #" + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp(), getClients());
	} else {									//send back topic if set - maybe this isn't needed for implementation for irssi since the client
		if (_channels[channelName].hasTopic()) {//never asks for a set topic but returns the internal one from join or last topic change
			c->sendToClient(c->getColHost() + " 332 " + c->getNickname() + " #" + channelName + " :" + _channels[channelName].getTopic());
			c->sendToClient(c->getColHost() + " 333 " + c->getNickname() + " #" + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp());
		} else c->sendToClient(c->getColHost() + " 331 :No topic");
	}
	return (true);
}

bool	Server::user(std::string &line, std::vector<Client>::iterator c) {
	std::string username, realname, hostname, servername, dummy;
	std::stringstream streamline;
	streamline << line;
	std::getline(streamline, username, ' ');
	std::getline(streamline, hostname, ' ');
	std::getline(streamline, servername, ' ');
	std::getline(streamline, dummy, ':');
	std::getline(streamline, realname, '\0');
	// std::cout << "getIsPasswordValid: "<< c->getIsPasswordValid() << std::endl;
	if (c->getIsPasswordValid() == false)
		return (false);
	if (c->getIsRegistered())
		return (c->sendToClient(c->getColNick() + " 462 :You may not reregister"), false);
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
		return (c->sendToClient(c->getColNick() + " 461 USER :Not enough parameters"), false);
	if (username.length() > 30)
		username = username.substr(0, 30);
	if (hostname.length() > 63 || realname.length() > 128)
		return (c->sendToClient(c->getColNick() + " 432 :Erroneous USER parameters"), false);
	std::string illegal = " \r\n:";
	if (username.find_first_of(illegal) != std::string::npos || hostname.find_first_of(illegal) != std::string::npos || servername.find_first_of(illegal) != std::string::npos)
		return (c->sendToClient(c->getColNick() + " 432 :Erroneous username"), false);
	illegal = "\r\n:";
	if (realname.find_first_of(illegal) != std::string::npos)
		return (c->sendToClient(c->getColNick() + " 432 :Erroneous username"), false);

	c->setUsername(username);
	c->setHostname(hostname);
	c->setServername(servername);
	c->setRealname(realname);
	c->setIsUserComplete(true);
	if (c->getCapNegotiation() && c->getIsNickValid() && !c->getIsRegistered())
	{
		c->setIsRegistered(true);
		c->sendToClient(":" + _serverName + " 001 " + c->getNickname() + " :Welcome to the Internet Relay Network " + c->getNickname() + "!" + c->getUsername() + "@" + c->getHostname());
		// std::cout << "User registered: " << username << std::endl;
		return (true);
	}
	return (false);
}

bool	Server::quit(std::string &line, std::vector<Client>::iterator c)
{
	std::vector<std::string> toPart;
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
		if (it->second.isMember(c->getNickname())) {
			std::string line_substr;
			if (!line.length())
				line_substr = "";
			else
				line_substr = line.substr(1);
			std::string call = "#" + (*it).first + " :" + line_substr;
			//std::string call = "#" + (*it).first + " :" + line.substr(1);
			toPart.push_back(call);
		}
	for (std::vector<std::string>::iterator it = toPart.begin(); it != toPart.end(); it++)
		part(*it, c);
	quit_client(c->getFd());
	return true;
}
