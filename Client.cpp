#include "irc.hpp"
#include <iostream>
#include <unistd.h>

Client::Client(int fd_, const std::string& host_ip) : fd(fd_), host(host_ip), authenticated(false) , pass_ok(false){}

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