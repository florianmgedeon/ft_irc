#include "../inc/Server.hpp"
typedef std::map<std::string, bool(Server::*)(std::string&, Client&)>::iterator commandIter;
//typedef std::map<std::string, bool(Server::*)(std::string&, Client&)> commandMap;

commandIter	Server::checkCmd(std::string &line) {
//	if (line[0] == ':') // ignore source param for now
//		line = line.substr(line.find(" ") + 1);
	std::string find =line.substr(0, line.find(" ") + 1) ;
	return _commandMap.find(find);
}

bool	Server::parseClientInput(int fd, std::string buffer) {
//	std::cout << "in buffer: |" << buffer << "|" << std::endl;
	std::string line, dummy;
	std::stringstream streamline;
	streamline << buffer;
	bool res = true;
	while (res && std::getline(streamline, line, '\r')) {
		std::getline(streamline, dummy, '\n');
		commandIter comMapIt = checkCmd(line);
//		std::cout << "line: |" << line << "|" << std::endl;
		if (comMapIt != _commandMap.end())
			res = (this->*(comMapIt->second))(line, *getClient(fd));
	}
	return true;
}

//---------------------------COMMANDS------------------------------------------

bool	Server::cap(std::string &line, Client &c) {
//	std::cout << "CAP line: <" << line << ">" << std::endl;
	if (line.substr(0, 2).compare ("LS"))
		return (c.append_send_buffer(_serverName + ": CAP * LS"), true);
//	if (line.substr(0, 4).compare ("LIST"))
	if (line.substr(0, 4).compare ("REQ "))	//TODO: parse and actually do request
		return (c.append_send_buffer(_serverName + ": CAP * ACK " + *(line.begin() + 5)), true);
	if (line.substr(0, 3).compare ("END")) return true; //server doesn't msg back on CAP END
	return false;
}
//TODO: invite
bool	Server::invite(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::join(std::string &line, Client &c) {
	std::string pwds, readName, readPwd;
	std::stringstream pwdstream, chanNamestream;
	Channel ch;
	(void)c;
	if (line.find(' ')) {
		pwds = line.substr(line.find(' '));
		line = line.substr(0, line.find(' '));
	}
	pwdstream << pwds;
	chanNamestream << line;
	while (std::getline(chanNamestream, readName, ',')) {
		if (_channels.find(readName) == _channels.end()) {	//create channel
			if (readName[0] == '&') {	//create pw-locked channel
				std::getline(pwdstream, readPwd, ',');
				_channels.insert(std::make_pair(readName, Channel(readPwd)));
			} else if (readName[0] == '#') {	//create open chanel
				_channels.insert(std::make_pair(readName, Channel()));
			} else return (c.append_send_buffer(":" + readName + " 476 :Bad Channel Mask"), false);

		} else {	//try joining existing channel
			if (_channels[readName].isMemberBanned(&c)) {
				return (c.append_send_buffer(c.getColNick() + " " + readName + " 474 :Cannot join channel (+b)"), false);
			} else if (readName[0] == '&') {	//join pw-locked channel
				std::getline(pwdstream, readPwd, ',');
				if (!(_channels[readName].checkPassword(readPwd)))
					return (c.append_send_buffer(c.getColNick() + " " + readName + " 475 :Cannot join channel (+k)"), false);
				else {
//					_channels[readName]._members.push_back(&c); braucht an setter

				}
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
//TODO:list
bool	Server::list(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}
//TODO:mode
bool	Server::mode(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::names(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::nick(std::string &line, Client &c) {
	if (!line.size())
		return (c.append_send_buffer(c.getColNick() + " 431 " + c.getNickname() + " :No nickname given"), false);
	if (strchr("&#:", line[0]) || line.find_first_of(" \r\n") != std::string::npos || line.size() > 9)
		return (c.append_send_buffer(c.getColNick() + " 432 " + line + " :Erroneus nickname"), false);
	if (getClient(line) != _clients.end())
		return (c.append_send_buffer(c.getColNick() + " 433 " + c.getNickname() + " :Nickname is already in use"), false);
	for (size_t i = 0; i < _clients.size(); i++)
		_clients[i].append_send_buffer(c.getColNick() + " NICK " + line);
	c.setNickname(line);
	return true;
}

bool	Server::part(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::pass(std::string &line, Client &c) {
	if (!line.size())
		return (c.append_send_buffer(c.getColNick() + " 461 PASS :Not enough parameters"), false);
	if (c.getIsRegistered())
		return (c.append_send_buffer(c.getColNick() + " 462 :You may not reregister"), false);
	if (line == this->getPassword())
		return (c.setIsPasswordValid(true), true);
	else return (c.append_send_buffer(c.getColNick() + " 464 :Password incorrect"), false);
}

bool	Server::ping(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::pong(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::privmsg(std::string &line, Client &c) {
	std::string msg = line.substr(line.find(':') + 1);
	if (!msg.size())
		return (c.append_send_buffer(c.getColNick() + " 412 :No text to send"), false);
	line = line.substr(0, line.find(':'));
	if (!line.size())
		return (c.append_send_buffer(c.getColNick() + " 411 :No recipient given (PRIVMSG)"), false);
	bool toChannel = false;

	{//TODO: parse name parameter along commas for several recipients
		while (strchr("@%#&", line[0])) {
			if (line[0] == '@') line = line.substr(1); //TODO: send to channel ops
			else if (line[0] == '%') line = line.substr(1); //TODO: send to channel ops
			else if (line[0] == '#' || line[0] == '&') {
				line = line.substr(1);
				toChannel = true;
			}
		}

		if (toChannel) {
			if (_channels.find(line) == _channels.end())
				return (c.append_send_buffer(c.getColNick() + " 401 :No such channel") ,false);
			_channels[line].sendChannelMessage(c.getNickname(), ":" + c.getNickUserHost() + " PRIVMSG " + msg);
		} else {
			std::vector<Client>::iterator recp = getClient(line);
			if (recp == _clients.end())
				return (c.append_send_buffer(c.getColNick() + " 401 :No such nick") ,false);
			(*recp).append_send_buffer(c.getColNick() + " PRIVMSG " + msg);
		}
	}
	return true;
}

bool	Server::topic(std::string &line, Client &c) {
	(void)line; (void)c;
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
//	std::cout << "Uname: " << username << " hostname: " << hostname << " servername: " << servername << " realname: " << realname << std::endl;
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty() ||
		username.size() > 9 || hostname.size() > 9)
		return (c.append_send_buffer(c.getColNick() + " 461 USER :Malformed parameters"), false);

	c.setUsername(username);
	c.setHostname(hostname);
	c.setServername(servername);
	c.setRealname(realname);
	c.setIsRegistered(true);

	c.append_send_buffer(":" + servername + " 001 " + c.getNickname() + " :Welcome to the Internet Relay Network " + c.getNickname() + "!" + username + "@" + hostname);
	return true;
}

