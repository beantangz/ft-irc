#include "irc.hpp"
#include <iostream>
#include <unistd.h>

Client::Client(int fd_) : fd(fd_), authenticated(false) {}

Client::~Client() {}

int Client::recv_data() {
	char buf[512];
	int ret = read(fd, buf, sizeof(buf));
	if (ret > 0)
		recv_buffer.append(buf, ret);
	return ret;
}

void Client::queue_send(const std::string &msg, struct pollfd *fds, int index) {
	send_buffer += msg;
	fds[index].events |= POLLOUT;
}


void Client::join_channel(Channel *ch) {
	channels.push_back(ch);
}

void Client::leave_channel(Channel *ch) {
	// remove from vector
}
