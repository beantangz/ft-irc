#include "irc.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <iostream>

Channel::Channel(const std::string &name_)
	: name(name_),
	  invite_only(false),
	  topic_op_only(false),
	  has_key(false),
	  key(""),
	  has_limit(false),
	  user_limit(0)
{
}

Channel::~Channel() {}

bool Channel::has_client(Client* c)
{
	for (size_t i =0; i < clients.size(); i++)
	{
		if (c == clients[i])
			return true;
	}
	return false;
}
void Channel::add_client(Client *c) 
{
	//pour eviter les doublons de clients
	if (std::find(clients.begin(), clients.end(), c) != clients.end())
		return;
	if (clients.empty())
		operators.push_back(c);

	clients.push_back(c);
	//le channel sautopush dans le vecteur de channel du client
	c->channels.push_back(this);
}


void Channel::remove_client(Client *c)
{
	clients.erase(std::remove(clients.begin(), clients.end(), c), clients.end());

	//si cest un operateur ca le tej
	operators.erase(std::remove(operators.begin(), operators.end(), c), operators.end());

	//enlever le channel de la liste de channel du client
	c->channels.erase(std::remove(c->channels.begin(), c->channels.end(), this), c->channels.end());

	//refoutre un op si yen a plus
	if (!clients.empty() && operators.empty())
		operators.push_back(clients[0]);
}

int find_index_in_fds(int fd, struct pollfd *fds, int nfds) {
	for(int i = 0; i < nfds; ++i) {
	if (fds[i].fd == fd)
		return i;
	}
	return -1;
}

void Channel::broadcast(Client *from, const std::string &msg, struct pollfd *fds, int index, int nfds) 
{
	(void)index;
	for (size_t i = 0; i < clients.size(); ++i)
	{
		Client *c = clients[i];
		if (c != from)
		{
			int idx = find_index_in_fds(c->fd, fds, nfds);
			c->queue_send(msg, fds, idx);
		}
	}
}

void Channel::addInvitation(Client* c)
{
	invited_clients.push_back(c);
}
bool Channel::isTopicProtected()
{
	if (topic_op_only)
		return true;
	else
		return false;
}
bool Channel::isInvited(Client *c)
{
	for (std::vector<Client*>::iterator it = invited_clients.begin();
	it != invited_clients.end(); it++)
	{
		if (c == *it)
			return true;
	}
	return false;
}
bool Channel::isOperator(Client *c)
{
	for (size_t i = 0; i < operators.size(); i++)
	{
		if (operators[i] == c)
			return true;
	}
	return false;
}
void Channel::addOperator(Client *c)
{
	for (size_t i = 0; i < operators.size(); i++)
	{
		if (operators[i] == c)
			return;
	}
	operators.push_back(c);
}

void Channel::removeOperator(Client *c)
{
	for (size_t i = 0; i < operators.size(); i++)
	{
		if (operators[i] == c)
		{
			//erase ne prend que un it en param
			operators.erase(operators.begin() + i);
			return ;
		}
	}
}
void Channel::debug_print() const
{
    std::cout << "=== Channel: " << name << " ===" << std::endl;

    std::cout << "Clients: ";
    for (size_t i = 0; i < clients.size(); ++i)
        std::cout << clients[i]->nick << " ";
    std::cout << std::endl;

    std::cout << "Operators: ";
    for (size_t i = 0; i < operators.size(); ++i)
        std::cout << operators[i]->nick << " ";
    std::cout << std::endl;

    std::cout << "====================" << std::endl;
}