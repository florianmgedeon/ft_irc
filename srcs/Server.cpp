#include "../inc/Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password)
{   
    _running = true;
}

Server::~Server()
{
}

int Server::getPort() const
{
    return _port;
}

std::string Server::getPassword() const
{
    return _password;
}

void Server::setRunning(bool running)
{
    _running = running;
}


void Server::ft_send(int fd, std::string message)
{
    Client &client = _clients[fd];
    client.send_buffer += message + "\r\n";
    client.setWrite(true);
    for (nfds_t i = 0; i < _nfds; ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds[i].events |= POLLOUT;
            break;
        }
    }
}


void Server::create_command(int fd, char *buffer)
{
    Client &client = _clients[fd];
    client.append_send_buffer(buffer);


    size_t end_pos = client.send_buffer.find("\r\n");
    while (end_pos != std::string::npos && end_pos < 512)
    {
        std::string complete_cmd = client.send_buffer.substr(0, end_pos);
        client.send_buffer.erase(0, end_pos + 2);
        Command command(complete_cmd, &client);
        find_command(command);
        if (_clients.find(fd) == _clients.end())
            break;
        end_pos = client.send_buffer.find("\r\n");
    }
}

void Server::find_command(Command command)
{
    std::string cmd = command.getCommand();
    // if(cmd == "PING")
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



//============================================ poll loop ====================================//



void Server::handle_send(int index)
{
    Client &client = _clients[_pollfds[index].fd];
    if (!client.send_buffer.empty())
    {
        int bytes_sent = send(client.getFd(), client.send_buffer.c_str(), client.send_buffer.size(), MSG_NOSIGNAL);
        if (bytes_sent > 0)
        {
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
    _clients[client_fd] = Client(hostname, client_fd);
    if (_nfds >= SOMAXCONN)
        throw std::runtime_error("Too many clients");
    _pollfds[_nfds].fd = client_fd;
    _pollfds[_nfds].events = POLLIN | POLLHUP;
    _nfds++;
    std::cout << "Client connected" << std::endl;
}

void Server::recv_client(int index)
{
    char    buffer[512];
    int     bytes_received;
    int     client_fd = _pollfds[index].fd;

    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            std::cout << "Client disconnected" << std::endl;
        else
            throw std::runtime_error("recv() failed");
    }
    else
        std::cout << "Received: " << buffer << std::endl;
        //create_command(client_fd, buffer);
}

void Server::quit_client(int index)
{
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); )
    {
        if (it->second.isMember(&_clients[_pollfds[index].fd]))
        {
            it->second.removeMember(&_clients[_pollfds[index].fd]);
            ft_send(_pollfds[index].fd, "Client disconnected");
            if (it->second.isEmpty())
                _channels.erase(it++);
            else
                ++it;
        }
        else
            ++it;
    }
    _clients.erase(_pollfds[index].fd);
    close(_pollfds[index].fd);
    _pollfds[index] = _pollfds[_nfds - 1];
    _pollfds[index].events = POLLIN | POLLHUP;
    _pollfds[_nfds - 1].fd = -1;
    _nfds--;
}

void Server::start()
{
    ft_socket();
    while(_running)
    {
        if (poll(_pollfds, _nfds, -1) == -1)
        throw std::runtime_error("poll() failed");
        for (nfds_t i = 0; i < _nfds; ++i)
        {
            try
            {
                if (_pollfds[i].revents & POLLIN)
                {
                    if (_pollfds[i].fd == _serverSocketFd)
                    accept_client();
                    else
                    recv_client(i);
                }
                else if (_pollfds[i].revents & POLLOUT)
                handle_send(i);
                else if (_pollfds[i].revents & POLLHUP)
                quit_client(i);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: poll loop" << std::endl;
            }
            std::cout << "Waiting for events..." << std::endl;
        }
    }
    close(_serverSocketFd);
}