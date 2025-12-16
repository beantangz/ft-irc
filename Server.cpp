#include "irc.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

#include "Server.hpp"

#define MAX_CLIENTS 1024


Server::Server(int port, const std::string &pwd) : password(pwd) {
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) { perror("socket"); exit(1); }

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(listen_fd, 10) < 0) {
		perror("listen");
		exit(1);
	}

	std::cout << "Server listening on port " << port << std::endl;
}

Server::~Server() {
	close(listen_fd);
}

Channel* Server::find_channel(const std::string &name) {
	for (Channel *ch : channels) {
		if (ch->name == name)
			return ch;
	}
	Channel *newCh = new Channel(name);
	channels.push_back(newCh);
	return newCh;
}

void Server::command_NICK(Client *c, std::string nickname){
	if (nickname.empty()) {
		numeric_431(c);
		return;
	}
	bool taken = false;
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->nick == nickname) {
			taken = true;
			break;
		}
	}
	if (taken) {
		numeric_433(c, nickname);
		return;
	}
	c->nick = nickname;
	numeric_001(c);
}

void Server::command_JOIN(Client *c, std::string channel_name){
	if (channel_name.empty()) {
			c->send_data("ERROR :No channel name given\r\n");
			return;
		}
		Channel* ch = find_channel(channel_name);
		ch->add_client(c);
		c->channels.push_back(ch);

		std::string join_msg = ":" + c->nick + " JOIN :" + channel_name + "\r\n";
		ch->broadcast(c, join_msg);
}

void Server::handleBuffer(Client* c) {
	size_t pos;

	while ((pos = c->buffer.find("\r\n")) != std::string::npos) {
		std::string line = c->buffer.substr(0, pos);
		c->buffer.erase(0, pos + 2);

		if (line.empty())
			continue;

		handleCommand(c, line);
	}
}

void Server::handleCommand(Client* c,std::string& line)
{
	std::istringstream iss(line);
	std::string cmd;
	iss >> cmd;

	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	if (cmd == "NICK"){
		std::string nickname;
		iss >> nickname;
		command_NICK(c, nickname);
	}
	else if (cmd == "USER") {
		std::string username, unused, mode, realname;
		iss >> username >> unused >> mode;
		std::getline(iss, realname);
		if (!realname.empty() && realname[0] == ' ')
			realname.erase(0, 1);

		c->user = username;
		c->authenticated = true;
		c->send_data("USER command accepted\r\n");
	}
	else if (cmd == "JOIN") {
		std::string channel_name;
		iss >> channel_name;
		command_JOIN(c, channel_name);
	}
	else {
		send_numeric(c, "ft_irc", 421, c->nick, "Unknown command");
	}
}

void Server::run() {
	struct pollfd fds[MAX_CLIENTS];
	int nfds = 1;

	fds[0].fd = listen_fd;
	fds[0].events = POLLIN;

	while (true) {
		int ret = poll(fds, nfds, -1);
		if (ret < 0) {
			perror("poll");
			break;
		}

		if (fds[0].revents & POLLIN) {
			sockaddr_in client_addr;
			socklen_t addrlen = sizeof(client_addr);
			int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
			if (client_fd < 0) {
				perror("accept");
			} else {
				std::cout << "Nouveau client connecté: fd=" << client_fd << std::endl;

				Client *c = new Client(client_fd);
				clients.push_back(c);

				fds[nfds].fd = client_fd;
				fds[nfds].events = POLLIN;
				nfds++;
			}
		}

		//verif clients existant
		for (int i = 1; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				Client *c = nullptr;

				for (auto client : clients) {
					if (client->fd == fds[i].fd) {
						c = client;
						break;
					}
				}
				if (!c) continue;

				char buf[512];
				int bytes = c->recv_data();

				if (bytes <= 0) 
				{
					std::cout << "Client déconnecté: fd=" << c->fd << std::endl;
					close(c->fd);
					for (int j = i; j < nfds - 1; j++)
						fds[j] = fds[j + 1];
					nfds--;
					std::vector<Client*>::iterator it = std::find(clients.begin(), clients.end(), c);
					if (it != clients.end())
						clients.erase(it);
					delete c;
					i--;
					continue;
				}
				else
					handleBuffer(c);
			}
		}
	}
}