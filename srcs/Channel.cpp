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
            break;
        }
    }
}

bool Channel::isEmpty()
{
    return _members.empty();
}