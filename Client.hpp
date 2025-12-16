#pragma once

#include "irc.hpp"

class Client {
public:
	int				 fd;
	std::string		 nick;
	std::string		 user;
	std::string		 buffer;
	std::vector<Channel*> channels;
	bool				authenticated;

	Client(int fd_);
	~Client();

	int recv_data();

	void send_data(const std::string &msg);
	void join_channel(Channel *ch);
	void leave_channel(Channel *ch);
};