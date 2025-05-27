#include "../inc/Server.hpp"
typedef std::map<std::string, bool(Server::*)(std::string&, Client&)>::iterator commandIter;

bool	Server::parseClientInput(int fd, std::string buffer) {
	std::string line, dummy;
	std::stringstream streamline;
	streamline << buffer;
//	bool res = true;
	while (std::getline(streamline, line, '\r')) {
		std::getline(streamline, dummy, '\n');
		std::string find = line.substr(0, line.find(" ") + 1) ;
		transform(find.begin(), find.end(), find.begin(), ::toupper);
		commandIter comMapIt = _commandMap.find(find);	//find upcased command string in command map
		if (comMapIt != _commandMap.end()) {
			line = line.substr(comMapIt->first.size());	//advance line string by cmd size
			/*res = */(this->*(comMapIt->second))(line, *getClient(fd));	//execute command
		} else {
			Client c = *getClient(fd);
			c.sendToClient(c.getColNick() + " "+ find + " :Unknown command");
//			return false;
		}
	}
	return true;
}

//---------------------------COMMANDS------------------------------------------

bool	Server::cap(std::string &line, Client &c) {
//	std::cout << "CAP line: <" << line << ">" << std::endl;
	if (line.substr(0, 2) == "LS")
		return (c.setCapNegotiation(true), c.sendToClient(":" + _serverName + " CAP * LS :multi-prefix"), true);
//	if (line.substr(0, 4).compare ("LIST"))
	if (line.substr(0, 4) == "REQ ")//TODO: parse and actually do request
		return (c.sendToClient(":" + _serverName + " CAP * ACK :multi-prefix"), true);
	if (line.substr(0, 3) == "END")
		return (true);
	return false;
}
//TODO: invite
bool	Server::invite(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

void	Server::join_channel(std::string &channelName, Client &c, bool makeOp) {
	_channels[channelName].addMember(&c, makeOp);
	_channels[channelName].sendChannelMessage(c.getNickname(), c.getColNick() + " JOIN " + channelName);
	topic(channelName, c);
	names(channelName, c);
}

bool	Server::join(std::string &line, Client &c) {
//	std::cout << "join inb4: |" << line << "|" << std::endl;
	std::string pwds, readName, readPwd;
	std::stringstream pwdstream, chanNamestream;
	if (line.find(' ') != std::string::npos) {
		pwds = line.substr(line.find(' '));
		line = line.substr(0, line.find(' '));
		pwdstream << pwds;
	}
	chanNamestream << line;
	while (std::getline(chanNamestream, readName, ',')) {
		if (_channels.find(readName) == _channels.end()) {	//create channel
			if (readName[0] == '&') {	//create pw-locked channel
				std::getline(pwdstream, readPwd, ',');
				std::cout << "creating locked channel " << readName << " pwd " << readPwd << std::endl;
				_channels.insert(std::make_pair(readName, Channel(readPwd)));
				join_channel(readName, c, true);
			} else if (readName[0] == '#') {	//create open chanel
				std::cout << "creating open channel " << readName << std::endl;
				_channels.insert(std::make_pair(readName, Channel()));
				join_channel(readName, c, true);
			} else return (c.sendToClient(":" + readName + " 476 :Bad Channel Mask"), false);

		} else {	//try joining existing channel
			if (_channels[readName].isMemberBanned(c.getNickname())) {
				return (c.sendToClient(c.getColNick() + " " + readName + " 474 :Cannot join channel (+b)"), false);
			} else if (readName[0] == '&') {	//join pw-locked channel
				std::getline(pwdstream, readPwd, ',');
				if (!(_channels[readName].checkPassword(readPwd)))
					return (c.sendToClient(c.getColNick() + " " + readName + " 475 :Cannot join channel (+k)"), false);
				else {
					join_channel(readName, c, false);
				}
			} else if (readName[0] == '#') {	//join open channel
				join_channel(readName, c, false);
			}
		}
	}

	return (true);
}
//TODO:kick
bool	Server::kick(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

//TODO:mode
bool	Server::mode(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::names(std::string &line, Client &c) {
//	std::cout << "line: |" << line << "|" << std::endl;
	std::stringstream linestream;
	std::string channelName, res = c.getColHost() + " 353 " + c.getNickname() + " = " + line + " :";
	linestream << line;
	int first = 0;
	while (std::getline(linestream, channelName, ',')) {
		if (first++)
			res += ',';
		res += _channels[channelName].memberlist();
	}
	c.sendToClient(res);
	c.sendToClient(c.getColHost() + " 366 " + c.getNickname() + " " + channelName + " :End of NAMES list");
	return true;
}

bool	Server::nick(std::string &line, Client &c) {
	if (c.getIsPasswordValid() == false)
		return (false);
	if (!line.size())
		return (c.sendToClient(c.getColNick() + " 431 " + c.getNickname() + " :No nickname given"), false);

	const std::string specials = "[]\\`_^{|}";
	if (line.length() > 30 || (!isalpha(line[0]) && specials.find(line[0]) == std::string::npos) || line.find_first_of(" \r\n") != std::string::npos)
		return (c.sendToClient(c.getColNick() + " 432 " + line + " :Erroneous nickname"), false);
    for (size_t j = 1; j < line.length(); ++j)
    {
        char x = line[j];
        if (!isalnum(x) && specials.find(x) == std::string::npos)
			return (c.sendToClient(c.getColNick() + " 432 " + line + " :Erroneous nickname"), false);
    }
    if (line.find_first_of("0123456789") != std::string::npos && line.find_first_not_of("0123456789") == std::string::npos)
		return (c.sendToClient(c.getColNick() + " 432 " + line + " :Erroneous nickname"), false);

	if (getClient(line) != _clients.end())
		return (c.sendToClient(c.getColNick() + " 433 " + c.getNickname() + " :Nickname is already in use"), false);
	c.setNickname(line);
	c.setIsNickValid(true);
	for (size_t i = 0; i < _clients.size(); i++)
		_clients[i].sendToClient(c.getColNick() + " NICK " + line);
	// std::cout << "Nickname set to: " << line << std::endl;
	if (c.getIsUserComplete() && !c.getIsRegistered())
	{
		c.setIsRegistered(true);
		c.sendToClient(":" + c.getServername() + " 001 " + c.getNickname() + " :Welcome to the Internet Relay Network " + c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname());
		std::cout << "User registered: " << c.getUsername() << std::endl;
	}
	return true;
}

bool	Server::part(std::string &line, Client &c) {
	if (!line.size())
		return (c.sendToClient(c.getColNick() + " 461 PART :Not enough parameters"), false);
	std::stringstream linestream;
	std::string channelName, channels, reason;
	
	channels = line.substr(0, line.find(' '));
	reason = line.substr(line.find(':') + 1);
	std::cout << "line: |" << line << "|" << std::endl;
	linestream << channels;
	
	while (std::getline(linestream, channelName, ',')) {
		if (!channelExists(channelName))
			c.sendToClient(c.getColNick() + " 403 :No such channel");
		else if (!_channels[channelName].isMember(c.getNickname()))
			c.sendToClient(c.getColNick() + " 442 " + channelName + " :You're not on that channel");
		else {
			std::cout << "removing " << c.getNickname() << " r sz " << reason.size() << std::endl;
			_channels[channelName].sendChannelMessage(c.getNickname(), c.getNickUserHost() + " PART " + channelName + " :" + reason);
			c.sendToClient(c.getNickUserHost() + " PART " + channelName + " :" + reason);
			_channels[channelName].removeMember(c.getNickname());
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

bool	Server::pass(std::string &line, Client &c) {
	if (c.getCapNegotiation() == false)
		return (c.sendToClient(c.getColNick() + " 421 PASS :CAP negotiation not finished"), false);
	if (!line.size())
		return (c.sendToClient(c.getColNick() + " 461 PASS :Not enough parameters"), false);
	if (c.getIsRegistered())
		return (c.sendToClient(c.getColNick() + " 462 :You may not reregister"), false);
	if (line == this->getPassword())
		return (c.setIsPasswordValid(true), true);
	else 
	{
		c.sendToClient(c.getColNick() + " 464 :Password incorrect");
		quit_client(getIndexofClient(c.getFd()));
		return (false);
	}
}

bool	Server::ping(std::string &line, Client &c)//for when server gets ping from client
{
	if (line.empty())
		return (c.sendToClient(c.getColNick() + " 409 :No origin specified"), false);
	c.sendToClient(":" + _serverName + " PONG " + line);
	return (true);
}

bool	Server::pong(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::privmsg(std::string &line, Client &c) {
	std::cout << "line; |" << line << "|" << std::endl;
	std::string msg = line.substr(line.find(':') + 1);
	if (!msg.size())
		return (c.sendToClient(c.getColNick() + " 412 :No text to send"), false);
	line = line.substr(0, line.find(':') - 1);
	if (!line.size())
		return (c.sendToClient(c.getColNick() + " 411 :No recipient given (PRIVMSG)"), false);
	bool toChannel = false;

	{//TODO: parse name parameter along commas for several recipients
		while (strchr("@%", line[0])) {
			if (line[0] == '@') line = line.substr(1); //TODO: send to channel ops
			else if (line[0] == '%') line = line.substr(1); //TODO: send to channel ops
		}
		if (line[0] == '#' || line[0] == '&')
				toChannel = true;
		if (toChannel) {
			if (_channels.find(line) == _channels.end())
				return (c.sendToClient(c.getColNick() + " 401 :No such channel"), false);
			_channels[line].sendChannelMessage(c.getNickname(), ":" + c.getNickUserHost() + " PRIVMSG " + line + " :" + msg);
		} else {
			std::vector<Client>::iterator recp = getClient(line);
			if (recp == _clients.end())
				return (c.sendToClient(c.getColNick() + " 401 :No such nick"), false);
			std::cout << c.getNickname() << " sending privmsg to " << (*recp).getNickname() << ": " << msg << std::endl;
			(*recp).sendToClient(c.getColNick() + " PRIVMSG " + (*recp).getNickname() + " :" + msg);
		}
	}
	return true;
}

bool	Server::topic(std::string &line, Client &c) {
	if (!line.size())
		return (c.sendToClient(c.getColNick() + " 461 TOPIC :Not enough parameters"), false);
	std::string channelName, newTopic;
	channelName = line.substr(0, line.find(' '));
	if (!channelExists(channelName))
		return (c.sendToClient(c.getColNick() + " 403 :No such channel"), false);
	if (line.find(' ') != std::string::npos) {	//set new topic
		if (!_channels[channelName].isOperator(c.getNickname()))
			return (c.sendToClient(c.getColNick() + " 482 " + channelName + " :You're not channel operator"), false);
		newTopic = line.substr(line.find_last_of(':') + 1);
//		std::cout << "chanName: |" << channelName << "| newTopic: |" << newTopic << "|" << std::endl;
		_channels[channelName].setTopic(newTopic, c.getNickname());
		c.sendToClient(c.getColHost() + " 332 " + c.getNickname() + " " + channelName + " :" + _channels[channelName].getTopic());
		c.sendToClient(c.getColHost() + " 333 " + c.getNickname() + " " + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp());
	} else {
		if (_channels[channelName].hasTopic()) {	//send back topic if set
			c.sendToClient(c.getColHost() + " 332 " + c.getNickname() + " " + channelName + " :" + _channels[channelName].getTopic());
			c.sendToClient(c.getColHost() + " 333 " + c.getNickname() + " " + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp());
		} else c.sendToClient(c.getColHost() + " 331 :No topic");
	}
	return (true);
}

bool	Server::user(std::string &line, Client &c) {
	std::string username, realname, hostname, servername, dummy;
	std::stringstream streamline;
	streamline << line;
	std::getline(streamline, username, ' ');
	std::getline(streamline, hostname, ' ');
	std::getline(streamline, servername, ' ');
	std::getline(streamline, dummy, ':');
	std::getline(streamline, realname, '\0');
	// std::cout << "getIsPasswordValid: "<< c.getIsPasswordValid() << std::endl;
	if (c.getIsPasswordValid() == false)
		return (false);
	if (c.getIsRegistered())
		return (c.sendToClient(c.getColNick() + " 462 :You may not reregister"), false);
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
		return (c.sendToClient(c.getColNick() + " 461 USER :Not enough parameters"), false);
	if (username.length() > 30 || hostname.length() > 63 || realname.length() > 128)
		return (c.sendToClient(c.getColNick() + " 432 :Erroneous USER parameters"), false);
	std::string illegal = " \r\n:";
	if (username.find_first_of(illegal) != std::string::npos)
		return (c.sendToClient(c.getColNick() + " 432 :Erroneous username"), false);

	c.setUsername(username);
	c.setHostname(hostname);
	c.setServername(servername);
	c.setRealname(realname);
	c.setIsUserComplete(true);
	// std::cout << "Username set to: " << username << std::endl;
	if (c.getIsNickValid() && !c.getIsRegistered())
	{
		c.setIsRegistered(true);
		c.sendToClient(":" + c.getServername() + " 001 " + c.getNickname() + " :Welcome to the Internet Relay Network " + c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname());
		std::cout << "User registered: " << username << std::endl;
		return (true);
	}
	return (false);
}
