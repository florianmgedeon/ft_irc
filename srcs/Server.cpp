#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password)
{   
    _running = true;
    _serverName = "ircserv";
    _serverSocketFd = -1;
    _nfds = 0;
}

Server::~Server()
{
}

int Server::getPort() const
{
    return _port;
}

const std::string &Server::getPassword() const
{
    return _password;
}

void Server::setRunning(bool running)
{
    _running = running;
}

std::vector<Client>::iterator 	Server::getClient(int fd) {
	std::vector<Client>::iterator i = _clients.begin();
	for (; i != _clients.end(); i++)
		if ((*i).getFd() == fd)
			break;
	return i;
}

std::vector<Client>::iterator 	Server::getClient(const std::string nickname) {
	std::vector<Client>::iterator i = _clients.begin();
	for (; i != _clients.end(); i++) {
		std::string temp = (*i).getNickname();
		if (!temp.compare(nickname.c_str()))
			break;
	}
	return i;
}

void Server::ft_send(int fd, std::string message)
{
    Client &client = *(getClient(fd));
    if (client.getFd() == -1)
        return;
    client.send_buffer.clear();
    client.send_buffer += message + "\r\n";
    client.setWrite(true);
    for (nfds_t i = 0; i < _nfds; ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            if (_pollfds[i].events & POLLOUT)
                return;
            _pollfds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::create_command(int fd, char *buffer)
{
    Client &client = *(getClient(fd));
	client.append_recv_buffer(buffer);

	size_t end_pos = client.recv_buffer.find("\r\n");
	while (end_pos != std::string::npos && end_pos < 512)
	{
		std::string complete_cmd = client.recv_buffer.substr(0, end_pos);
		client.recv_buffer.erase(0, end_pos + 2);
		Command command(complete_cmd, &client);
		find_command(command);
		if (getClient(fd) == _clients.end())
			break;
		end_pos = client.recv_buffer.find("\r\n");
	}
}

void Server::find_command(Command command)
{
    std::string cmd = command.getCommand();
    if (cmd == "CAP")
    {
        cap_command(command);
    }
    else if (cmd == "PASS")
    {
        pass_command(command);
    }
    else if (cmd == "NICK")
    {
        nick_command(command);
    }
    else if (cmd == "USER")
    {
        user_command(command);
    }
    // else if(cmd == "PING")
    //     ping_command(command);
    // else if(cmd == "PONG")
    //     pong_command(command);
	// else if (cmd == "KICK")
	// 	kick_command(command);
	// else if (cmd == "INVITE")
	// 	invite_command(command);
	// else if (cmd == "TOPIC")
	// 	topic_command(command);
	// else if (cmd == "MODE")
	// 	mode_command(command);
    // else {}
        // Handle unknown command
}

//============================================ commands ====================================//

void Server::numeric_reply(int fd, const std::string& code, const std::string& target, const std::string& msg, int index)
{
    std::string response = ":" + _serverName + " " + code + " " + target + " :" + msg;
    ft_send(fd, response);
    if (index == -1)
        return;
    handle_send(index);
}

//cap_command
void Server::cap_command(Command command)
{
    if (command.getParams()[0] == "LS")
    {
        Client &client = *command.getClient();

        std::string cap_response = ":" + _serverName + " CAP * LS :multi-prefix\r\n";
        ft_send(client.getFd(), cap_response);
        nfds_t i = 1;
        while (i < _nfds)
        {
            if (_pollfds[i].fd == client.getFd())
                break;
            i++;
        }
        handle_send(i);
        return;
    }

    if (command.getParams()[0] == "REQ")
    {
        Client &client = *command.getClient();
        std::string cap_response = ":" + _serverName + " CAP * ACK :multi-prefix\r\n";
        ft_send(client.getFd(), cap_response);
        nfds_t i = 1;
        while (i < _nfds)
        {
            if (_pollfds[i].fd == client.getFd())
                break;
            i++;
        }
        handle_send(i);
        return;
    }

    if (command.getParams()[0] == "END")
    {
        Client &client = *command.getClient();
        client.setCapNegotiation(true);
        return;
    }
}

void Server::pass_command(Command command)
{
    std::string password = command.getParams()[0];
    Client &client = *command.getClient();
    nfds_t i = 1;
    while (i < _nfds)
    {
        if (_pollfds[i].fd == client.getFd())
            break;
        i++;
        if (i == _nfds)
            i = -1;
    }
    if (password.empty())
    {
        numeric_reply(client.getFd(), "461", "PASS", "Not enough parameters", i);
        return;
    }
    if (client.getIsRegistered())
    {
        numeric_reply(client.getFd(), "462", "*", "You may not reregister", i);
        return;
    }
    if (password != _password)
    {
        numeric_reply(client.getFd(), "464", "*", "Password incorrect", i);
        quit_client(i);
        return;
    }
    client.setIsPasswordValid(true);
    std::cout << "Password is valid: " << password << std::endl;
}

void Server::nick_command(Command command)
{
    std::string nickname = command.getParams()[0];
    Client &client = *command.getClient();
    nfds_t i = 1;
    while (i < _nfds)
    {
        if (_pollfds[i].fd == client.getFd())
            break;
        i++;
        if (i == _nfds)
            i = -1;
    }
    if (!client.getIsPasswordValid())
    {
        numeric_reply(client.getFd(), "462", "*", "Unauthorized command (already registered)", i);
        return;
    }
    if (nickname.empty())
    {
        numeric_reply(client.getFd(), "431", "*", "No nickname given", i);
        return;
    }
    const std::string specials = "[]\\`_^{|}";
    char first = nickname[0];
    if (nickname.length() > 30 || (!isalpha(first) && specials.find(first) == std::string::npos) || nickname.find_first_of(" \r\n") != std::string::npos)
    {
        numeric_reply(client.getFd(), "432", nickname, "Erroneous nickname", i);
        return;
    }
    for (size_t j = 1; j < nickname.length(); ++j)
    {
        char c = nickname[j];
        if (!isalnum(c) && specials.find(c) == std::string::npos)
        {
            numeric_reply(client.getFd(), "432", nickname, "Erroneous nickname", i);
            return;
        }
    }
    if (nickname.find_first_of("0123456789") != std::string::npos && nickname.find_first_not_of("0123456789") == std::string::npos)
    {
        numeric_reply(client.getFd(), "432", nickname, "Erroneous nickname", i);
        return;
    }
    if (getClient(nickname) != _clients.end())
    {
        numeric_reply(client.getFd(), "433", nickname, "Nickname is already in use", i);
        return;
    }
    if (nickname == client.getNickname())
    {
        numeric_reply(client.getFd(), "462", "*", "You may not reregister", i);
        return;
    }
    client.setNickname(nickname);
    std::cout << "Nickname set to: " << nickname << std::endl;
}

void Server::user_command(Command command)
{
    std::string username = command.getParams()[0];
    std::string hostname = command.getParams()[1];
    std::string servername = command.getParams()[2];
    std::string realname = command.getParams()[3];
    Client &client = *command.getClient();

    //just use to silence the warning
    (void)client;
    (void)username;
    (void)hostname;
    (void)servername;
    (void)realname;

    /*
    if (!client.getIsPasswordValid())
    {
        numeric_reply(client.getFd(), "462", "*", "Unauthorized command (already registered)");
        return;
    }

    if (username.empty() || hostname.empty() || servername.empty() || realname.empty())
    {
        numeric_reply(client.getFd(), "461", "USER", "Not enough parameters");
        return;
    }

    if (username.length() > 9 || username.find_first_of(" \r\n") != std::string::npos || username[0] == '#')
    {
        numeric_reply(client.getFd(), "461", "USER", "Erroneous username");
        return;
    }
    if (hostname.length() > 9 || hostname.find_first_of(" \r\n") != std::string::npos || hostname[0] == '#')
    {
        numeric_reply(client.getFd(), "461", "USER", "Erroneous hostname");
        return;
    }
    if (servername.length() > 9 || servername.find_first_of(" \r\n") != std::string::npos || servername[0] == '#')
    {
        numeric_reply(client.getFd(), "461", "USER", "Erroneous servername");
        return;
    }
    if (realname.length() > 9 || realname.find_first_of(" \r\n") != std::string::npos || realname[0] == '#')
    {
        numeric_reply(client.getFd(), "461", "USER", "Erroneous realname");
        return;
    }

    client.setUsername(username);
    client.setHostname(hostname);
    client.setServername(servername);
    client.setRealname(realname);
    client.setIsRegistered(true);

    client.setWrite(true);
    client.send_buffer += ":" + servername + " 001 " + client.getNickname() + " :Welcome to the Internet Relay Network " + client.getNickname() + "!" + username + "@" + hostname + "\r\n";
    */
}






//============================================ poll loop ====================================//



void Server::handle_send(int index)
{
    Client &client = *(getClient(_pollfds[index].fd));
    if (!client.send_buffer.empty())
    {
        int bytes_sent = send(client.getFd(), client.send_buffer.c_str(), client.send_buffer.size(), MSG_NOSIGNAL);
        if (bytes_sent > 0)
        {
            std::cout << "buffer from send: " << client.send_buffer.c_str() << std::endl;
            client.send_buffer.erase(0, bytes_sent);
            if (client.send_buffer.empty())
            {
                client.setWrite(false);
                _pollfds[index].events = POLLIN | POLLHUP;
            }
        }
        else if (bytes_sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            quit_client(index);
    }
}

void Server::ft_socket()
{
    struct sockaddr_in server_addr;

    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("socket() failed");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_port = htons(_port); //port
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP address

    int opt = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt() failed");

    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");

    if (bind(_serverSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        throw std::runtime_error("bind() failed");

    if (listen(_serverSocketFd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");

    _nfds = 1;
    memset(_pollfds, 0, sizeof(_pollfds));
    _pollfds[0].fd = _serverSocketFd;
    _pollfds[0].events = POLLIN | POLLHUP | POLLOUT;
}

void Server::accept_client()
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(_serverSocketFd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd == -1)
        throw std::runtime_error("accept() failed");
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    std::string hostname(inet_ntoa(client_addr.sin_addr));
    _clients.push_back(Client(hostname, &_pollfds[_nfds]));
    if (_nfds >= SOMAXCONN)
        throw std::runtime_error("Too many clients");
    _pollfds[_nfds].fd = client_fd;
    _pollfds[_nfds].events = POLLIN | POLLHUP;
    _nfds++;
}

bool Server::recv_client(int index)
{
    char buffer[512];
    int client_fd = _pollfds[index].fd;
    // Client &client = _clients[client_fd];
    // if (!client.getIsRegistered())
    //     return false;
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received == 0)
        return quit_client(index); // true = client removed
    else if (bytes_received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        throw std::runtime_error("recv() failed");
    }
    else
    {
        std::cout << "buffer from recv: " << buffer << std::endl;
        create_command(client_fd, buffer);
        return false;
    }
}

bool Server::quit_client(int index)
{
    int fd = _pollfds[index].fd;

    if (getClient(fd) == _clients.end())
        return false; // already removed

    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); )
    {
        if (it->second.isMember(&_clients[fd]))
        {
            it->second.removeMember(&_clients[fd]);
            //ft_send(fd, "Client disconnected"); //broadcast?
            if (it->second.isEmpty())
                _channels.erase(it++);
            else
                ++it;
        }
        else
            ++it;
    }

//    _clients.erase(fd);
    close(fd);

    if (_nfds > 2)
    {
        _pollfds[index] = _pollfds[_nfds - 1];
        _pollfds[_nfds - 1].fd = -1;
        _pollfds[_nfds - 1].events = 0;
    }
    else if (_nfds == 2)
    {
        _pollfds[index].fd = -1;
        _pollfds[index].events = 0;
    }

    _nfds--;
    std::cout << "Client disconnected" << std::endl;
    return true;
}

void Server::start()
{
    ft_socket();
    std::cout << "Socket open, awaiting clients." << std::endl;
    while (_running)
    {
        if (poll(_pollfds, _nfds, -1) == -1)
            throw std::runtime_error("poll() failed");
        if (_pollfds[0].revents & POLLIN)
            accept_client();
        nfds_t i = 1;
        while (i < _nfds)
        {
            short revents = _pollfds[i].revents;
            bool client_removed = false;
            try
            {
                if (revents & POLLIN)
                    client_removed = recv_client(i);
                if (!client_removed && (revents & POLLOUT))
                    handle_send(i);
                if (!client_removed && (revents & POLLHUP))
                    client_removed = quit_client(i);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error in poll loop: " << e.what() << std::endl;
                client_removed = quit_client(i);
            }
            if (!client_removed)
                ++i;
        }
        //std::cout << "Number of clients: " << (_nfds - 1) << std::endl;
    }
    close(_serverSocketFd);
}
