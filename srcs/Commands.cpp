#include "../inc/Server.hpp"
typedef std::map<std::string, bool(Server::*)(std::string&, std::vector<Client>::iterator)>::iterator commandIter;
typedef std::map<std::string, Channel>::iterator channelIter;
//TODO: INVITE OPER MODE


//rest commands die net zu operator ghörn
//ACTION: wird vom client interpretiert und is parameter von privmsg -> funktioniert
//NOTICE: ka command sondern message die bots für kommunikation verwenden statt privmsg, brauch ma am end net bzw. nur fürn bonus?
//TIME: nice to have

std::string	tokenize(std::string &line, char c) {
	std::string res;
	if (line.find(' ')) {
		res = line.substr(0, line.find(c));
		line = line.substr(line.find(c) + 1);
	}
	return res;
}

std::string	strPastColon(std::string &line) {return line.substr(line.find(':') + 1);}

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
		return (/*c->setCapNegotiation(true), */c->sendToClient(":" + _serverName + " CAP * LS :multi-prefix"), true);
//	if (param == "LIST")
	if (param == "REQ")//TODO: parse and actually do request
		return (c->sendToClient(":" + _serverName + " CAP * ACK :multi-prefix"), true);
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

void	Server::join_channel(std::string &channelName, std::vector<Client>::iterator c) {
	_channels[channelName].addMember(c->getNickname());
	_channels[channelName].sendChannelMessage(c->getNickUserHost(), c->getColNick() + " JOIN " + channelName, getClients());
	topic(channelName, c);
	names(channelName, c);
}

bool	Server::join(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "join inb4: |" << line << "|" << std::endl;
	std::string readName, readPwd;
	std::stringstream pwdstream, chanNamestream;
	chanNamestream << tokenize(line, ' ');
	pwdstream << strPastColon(line);

	while (std::getline(chanNamestream, readName, ',')) {
		readName = readName.substr(1);
		if (_channels.find(readName) == _channels.end()) {	//create channel
			std::getline(pwdstream, readPwd, ',');
			if (readPwd.size()) {	//create pw-locked channel
				std::cout << "creating locked channel " << readName << " pwd " << readPwd << std::endl;
				_channels.insert(std::make_pair(readName, Channel(c->getNickname(), readPwd)));
				join_channel(readName, c);
			} else {	//create open chanel
				std::cout << "creating open channel " << readName << std::endl;
				_channels.insert(std::make_pair(readName, Channel(c->getNickname())));
				join_channel(readName, c);
			} //else return (c->sendToClient(":" + readName + " 476 :Bad Channel Mask"), false); //TODO: reimplement error

		} else {	//try joining existing channel
			std::getline(pwdstream, readPwd, ',');	//join pw-locked channel
			if (!(_channels[readName].checkPassword(readPwd)))
				return (c->sendToClient(c->getColNick() + " " + readName + " 475 :Cannot join channel (+k)"), false);
			join_channel(readName, c);
		}
	}
	return true;
}

bool	Server::kick(std::string &line, std::vector<Client>::iterator c) {
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 KICK :Not enough parameters"), false);
	std::string channel = tokenize(line, ' ');
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

	std::string channel = tokenize(line, ' ');
	std::string modestring = tokenize(line, ' ');
	std::string argument = tokenize(line, ' ');
//std::cout <<"channel: " <<channel <<std::endl;
//std::cout <<"modestring: " <<modestring <<std::endl;
//std::cout <<"argument: " <<argument <<std::endl;
	if (!channelExists(channel))
		return (c->sendToClient(c->getColNick() + 
			" 403 MODE:No such channel"), false);
	if (!modestring.size())
		return false; /*TODO: RPL_CHANNELMODEIS (324),
			RPL_CREATIONTIME (329)*/
	
	if (modestring.size() && !(_channels[channel].isOperator(c->getNickname())))
		return (c->sendToClient(c->getColNick() + " 482 " + channel
			+ " :You're not channel operator"), false);
	
	return _channels[channel].executeMode(modestring, argument);
}

bool	Server::names(std::string &line, std::vector<Client>::iterator c) {
//	std::cout << "line: |" << line << "|" << std::endl;
	std::stringstream linestream;
	std::string channelName, res;
	linestream << line;
	while (std::getline(linestream, channelName, ','))
		res += c->getColHost() + " 353 " + c->getNickname() + " = " + line + " :" + _channels[channelName].memberlist();
	c->sendToClient(res);
	c->sendToClient(c->getColHost() + " 366 " + c->getNickname() + " " + channelName + " :End of NAMES list");
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
	c->setNickname(line);
	for (size_t i = 0; i < _clients.size(); i++)
		_clients[i].sendToClient(":" + oldNick + " NICK :" + c->getNickname());

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
		if (!channelExists(channelName))
			c->sendToClient(c->getColNick() + " 403 :No such channel");
		else if (!_channels[channelName].isMember(c->getNickname()))
			c->sendToClient(c->getColNick() + " 442 " + channelName + " :You're not on that channel");
		else {
			std::cout << "removing " << c->getNickname() << " r sz " << reason.size() << std::endl;
			_channels[channelName].sendChannelMessage(c->getNickname(), c->getNickUserHost() + " PART " + channelName, getClients());
			c->sendToClient(c->getNickUserHost() + " PART " + channelName + " :" + reason);
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
			if (_channels.find(username) == _channels.end())
				return (c->sendToClient(c->getColNick() + " 401 :No such channel"), false);
			_channels[username].sendChannelMessage(c->getNickname(), c->getNickUserHost() + " PRIVMSG " + username + " :" + msg, getClients());
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
	if (!line.size())
		return (c->sendToClient(c->getColNick() + " 461 TOPIC :Not enough parameters"), false);
	std::string channelName = tokenize(line, ' ');
	if (!channelExists(channelName))
		return (c->sendToClient(c->getColNick() + " 403 :No such channel"), false);
	if (line.find(' ') != std::string::npos) {	//set new topic
		if (!_channels[channelName].isOperator(c->getNickname()))
			return (c->sendToClient(c->getColNick() + " 482 " + channelName + " :You're not channel operator"), false);
		line = strPastColon(line);
		_channels[channelName].setTopic(line, c->getNickname());
		_channels[channelName].sendChannelMessage("", c->getColHost() + " 332 " + c->getNickname() + " " + channelName + " :" + _channels[channelName].getTopic(), getClients());
		_channels[channelName].sendChannelMessage("", c->getColHost() + " 333 " + c->getNickname() + " " + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp(), getClients());
	} else {									//send back topic if set - maybe this isn't needed for implementation for irssi since the client
		if (_channels[channelName].hasTopic()) {//never asks for a set topic but returns the internal one from join or last topic change
			c->sendToClient(c->getColHost() + " 332 " + c->getNickname() + " " + channelName + " :" + _channels[channelName].getTopic());
			c->sendToClient(c->getColHost() + " 333 " + c->getNickname() + " " + channelName + " " + _channels[channelName].getTopicSetter() + " " + _channels[channelName].getTopicTimestamp());
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
	for (channelIter it = _channels.begin(); it != _channels.end(); it++)
		if (it->second.isMember(it->first))
		{
			std::string call = (*it).first + " " + line.substr(1);
			part(call, c);
		}
	quit_client(c->getFd());
	return true;
}
