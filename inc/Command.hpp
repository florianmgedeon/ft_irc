#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Server.hpp"
#include "Client.hpp"
#include <vector>

class Client;

class Command
{
    private:
        Client*                     _client;
        std::string                 _completeCommand;
        std::string                 _allParams;
        std::string                 _command;
        std::vector<std::string>    _paramsVector;

    public:
        Command();
        ~Command();
        Command(std::string complete_command, Client *client);
        std::string getCommand() const;
        Client*     getClient() const;
        void        parseCommand();
        void        parseParams();
};

void ping_command(Command command);
void pong_command(Command command);
void kick_command(Command command);
void invite_command(Command command);
void topic_command(Command command);
void mode_command(Command command);

#endif