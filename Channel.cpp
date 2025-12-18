#include "irc.hpp"
#include <algorithm>


Channel::Channel(const std::string &name_) : name(name_) {}
Channel::~Channel() {}

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

void Channel::broadcast(Client *from, const std::string &msg) {
	for (Client *c : clients) {
		if (c != from)
			c->queu_send(msg);
	}
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
			operators.erase(operators.begin() + 1);
			return ;
		}
	}
}