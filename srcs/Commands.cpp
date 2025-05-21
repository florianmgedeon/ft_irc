#include "../inc/Server.hpp"

static cmds checkCmd(std::string &line) {
	if (line[0] == ':') // ignore source param for now
		line = line.substr(line.find(" ") + 1);
	if (!line.substr(0, 4).compare("CAP "))
		return (line = line.substr(4), CAP);
	else if (!line.substr(0, 5).compare("PASS "))
		return (line = line.substr(5), PASS);
	else if (!line.substr(0, 5).compare("NICK "))
		return (line = line.substr(5), NICK);
	else if (!line.substr(0, 5).compare("USER "))
		return (line = line.substr(5), USER);
	return NONE;
}

bool	Server::cap(std::string &line, Client &c) {
//	std::cout << "line: <" << line << ">" << std::endl;
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

bool	Server::create_command(int fd, std::string buffer) {
//	std::cout << "in buffer: |" << buffer << "|" << std::endl;
	std::string line, dummy;
	std::stringstream streamline;
	streamline << buffer;
	while (std::getline(streamline, line, '\r')) {
		std::getline(streamline, dummy, '\n');
		cmds cmd = checkCmd(line);
//		std::cout << "line: |" << line << "|" << std::endl;
		switch (cmd) {
		//AUTH:
		case CAP : cap (line, *getClient(fd)); break;
		case PASS: pass(line, *getClient(fd)); break;
		case NICK: nick(line, *getClient(fd)); break;
		case USER: user(line, *getClient(fd)); break;
		case NONE: break;
		}

		//CHANNELS:
		//JOIN PART TOPIC NAMES LIST INVITE
		//MESSAGES:
		//PRIVMSG
	}
	return true;
}
