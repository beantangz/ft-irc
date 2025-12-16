#pragma once

#include "irc.hpp"

class Channel {

public:
	std::string		 name;
	std::vector<Client*> clients;

	Channel(const std::string &name_);
	~Channel();

	void add_client(Client *c);
	void remove_client(Client *c);
	void broadcast(Client *from, const std::string &msg);
};