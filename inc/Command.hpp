#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Server.hpp"
#include "Client.hpp"
#include <vector>
#include <sstream>

class Client;

class Command
{
    private:
        std::string                 _completeCommand;
        Client*                     _client;
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
        //getParams
        std::vector<std::string> getParams() const;
};

#endif