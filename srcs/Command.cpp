#include "Command.hpp"

Command::Command()
{
}

Command::Command(std::string complete_command, Client *client) : _completeCommand(complete_command), _client(client)
{
    parseCommand();
    parseParams();
}

Command::~Command()
{
}

std::string Command::getCommand() const
{
    return _command;
}

Client* Command::getClient() const
{
    return _client;
}

void Command::parseCommand()
{
    size_t start = _completeCommand.find_first_not_of(" \t\r\n");
    size_t end = _completeCommand.find(' ', start);
    if (start != std::string::npos)
        _command = _completeCommand.substr(start, end - start);
    for (size_t i = 0; i < _command.length(); ++i)
        _command[i] = std::toupper(_command[i]);
    _allParams = _completeCommand;
    _allParams.erase(0, end);
}

void Server::ping_command(Command command)
{
}

void Server::pong_command(Command command)
{
}

void Server::kick_command(Command command)
{
    //NEEDS: command_client(op?), kicked-client(params), channel(params)

}