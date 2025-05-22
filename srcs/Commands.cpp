#include "../inc/Server.hpp"
typedef std::map<std::string, bool(Server::*)(std::string&, Client&)>::iterator commandIter;
//typedef std::map<std::string, bool(Server::*)(std::string&, Client&)> commandMap;

commandIter	Server::checkCmd(std::string &line) {
	if (line[0] == ':') // ignore source param for now
		line = line.substr(line.find(" ") + 1);
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
		//CHANNELS:
		//JOIN PART TOPIC NAMES LIST INVITE
	}
	return true;
}

bool	Server::privmsg(std::string &line, Client &c) {
//	std::stringstream streamline;
	std::string msg = line.substr(line.find(':') + 1);
	line = line.substr(0, line.find(' '));
	bool toChannel = false;

	{//TODO: parse name parameter along commas for several recipients
		while (strchr("@%#", line[0])) {
	//		streamline << line;
			if (line[0] == '@') line = line.substr(1); //TODO: send to channel ops
			else if (line[0] == '%') line = line.substr(1); //TODO: send to channel ops
			else if (line[0] == '#') {
				line = line.substr(1);
				toChannel = true;
			}
		}
		if (toChannel) {
			if (_channels.find(line) != _channels.end())
				_channels[line].sendChannelMessage(c.getNickname(), msg);
		} else {
			Client &recp = *getClient(line);
			recp.append_send_buffer(c.getColNick() + " PRIVMSG " + msg);
		}
	}
	return true;
}

bool	Server::join(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::part(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::topic(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::names(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::list(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::invite(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}


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

bool	Server::pass(std::string &line, Client &c) {
	if (!line.size())
		return (c.append_send_buffer(c.getColNick() + " 461 PASS :Not enough parameters"), false);
	if (c.getIsRegistered())
		return (c.append_send_buffer(c.getColNick() + " 462 :You may not reregister"), false);
	if (line == this->getPassword())
		return (c.setIsPasswordValid(true), true);
	else return (c.append_send_buffer(c.getColNick() + " 464 :Password incorrect"), false);
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

bool	Server::ping(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::pong(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::kick(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}

bool	Server::mode(std::string &line, Client &c) {
	(void)line; (void)c;
	return (true);
}
