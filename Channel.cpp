#include "irc.hpp"
#include <algorithm>

Channel::Channel(const std::string &name_) : name(name_) {}
Channel::~Channel() {}

void Channel::add_client(Client *c) {
	if (clients.empty())
		modo = c;
	clients.push_back(c);
}

void Channel::remove_client(Client *c) {
	clients.erase(std::remove(clients.begin(), clients.end(), c), clients.end());
}

void Channel::broadcast(Client *from, const std::string &msg) {
	for (Client *c : clients) {
		if (c != from)
			c->queu_send(msg);
	}
}

