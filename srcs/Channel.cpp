#include "../inc/Channel.hpp"

Channel::Channel()
{
}

Channel::~Channel()
{
}

bool Channel::isMember(Client *client)
{
    for (std::list<Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}

void Channel::removeMember(Client *client)
{
    for (std::list<Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        if (*it == client)
        {
            _members.erase(it);
            if (isOperator(client))
                removeOperator(client);
            break;
        }
    }
}

bool Channel::isEmpty()
{
    return _members.empty();
}

bool Channel::isOperator(Client *client)
{
    for (std::list<Client *>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}

void Channel::addOperator(Client *client)
{
    if (isMember(client) && !isOperator(client))
        _operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
    for (std::list<Client *>::iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        if (*it == client)
        {
            _operators.erase(it);
            break;
        }
    }
}