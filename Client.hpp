#pragma once

#include "irc.hpp"
class Channel;

class Client {
public:
	int				 fd;
	std::string		 nick;
	std::string		 user;
	std::string		 host;

	std::string		recv_buffer;
	std::string		send_buffer;

	std::vector<Channel*> channels;
	bool				authenticated;
	bool				pass_ok;

	Client(int fd_, const std::string& host_ip);
	~Client();

	int recv_data();

	void queue_send(const std::string &msg, struct pollfd *fds, int index);
	//void join_channel(Channel *ch);
	void leave_channel(Channel *ch);
	
};