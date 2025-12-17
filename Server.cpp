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

void Server::command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds){
	if (channel_name.empty()) {
			c->queue_send("ERROR :No channel name given\r\n", fds, index);
			return;
		}
		Channel* ch = find_channel(channel_name);
		ch->add_client(c);
		c->channels.push_back(ch);

		std::string join_msg = ":" + c->nick + " JOIN :" + channel_name + "\r\n";
		ch->broadcast(c, join_msg);
}

void Server::handleBuffer(Client* c, int index, struct pollfd *fds) {
	size_t pos;

	while ((pos = c->recv_buffer.find("\r\n")) != std::string::npos) {
		std::string line = c->recv_buffer.substr(0, pos);
		c->recv_buffer.erase(0, pos + 2);

		if (line.empty())
			continue;

		handleCommand(c, line, index, fds);
	}
}



void Server::handleCommand(Client* c,std::string& line, int index, struct pollfd *fds)
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
		c->queue_send("USER command accepted\r\n", fds, index);
	}
	else if (cmd == "JOIN") {
		std::string channel_name;
		iss >> channel_name;
		command_JOIN(c, channel_name,index, fds);
	}
	else if (cmd == "MODE")
	{
		std::string target;
		std::string modes;
		std::string param;

		iss >> target >> modes;
		if (!iss.eof())
			iss >> param;
		command_MODE(c, target, modes, param, index, fds);
	}
	else {
		send_numeric(c, "ft_irc", 421, c->nick, "Unknown command");
	}
}

void Server::tchek_listen(int &nfds, struct pollfd *fds){

	if (fds[0].revents & POLLIN) {
			sockaddr_in client_addr;
			socklen_t addrlen = sizeof(client_addr);
			int client_fd = accept(fds[0].fd, (struct sockaddr*)&client_addr, &addrlen);
			if (client_fd < 0) {
				perror("accept");
				return;
			}
			if (nfds >= MAX_CLIENTS) {
				close(client_fd);
				return;
			}
			else {
				std::cout << "Nouveau client connecté: fd=" << client_fd << std::endl;

				Client *c = new Client(client_fd);
				clients.push_back(c);

				fds[nfds].fd = client_fd;
				fds[nfds].events = POLLIN;
				fds[nfds].revents = 0;
				nfds++;
			}
		}
}

Client* Server::find_client_by_fd(int fd) {
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->fd == fd)
			return clients[i];
	}
	return nullptr;
}

void Server::remove_client(int index, int &nfds, struct pollfd *fds) {
	Client *c = find_client_by_fd(fds[index].fd);
	if (!c) return;

	std::cout << "Client déconnecté: fd=" << c->fd << std::endl;
	close(c->fd);

	std::vector<Client*>::iterator it = std::find(clients.begin(), clients.end(), c);
	if (it != clients.end())
		clients.erase(it);

	delete c;

	for (int j = index; j < nfds - 1; j++)
		fds[j] = fds[j + 1];

	nfds--;
}

void Server::tchek_clients(int &nfds, struct pollfd *fds){
	for (int i = 1; i < nfds; i++) {
		if (fds[i].revents & (POLLHUP | POLLERR)) {
			remove_client(i, nfds, fds);
			i--;
			continue;
			}
		if (fds[i].revents & POLLIN) {
			Client *c = find_client_by_fd(fds[i].fd);
			if (!c) continue;
			int bytes = c->recv_data();
			if (bytes <= 0) 
			{
				remove_client(i, nfds, fds);
				i--;
			}
			else
				handleBuffer(c, i, fds);
			}
		}
}

void Server::tchek_clients_out(int nfds, struct pollfd *fds) {
	for (int i = 1; i < nfds; ++i) {
		Client *c = find_client_by_fd(fds[i].fd);
		if (!c)
			continue;

		if ((fds[i].revents & POLLOUT) && !c->send_buffer.empty()) {
			int sent = send(c->fd, c->send_buffer.c_str(), c->send_buffer.size(), 0);

			if (sent > 0) {
				c->send_buffer.erase(0, sent);
			}
			else if (sent == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
				std::cerr << "Erreur d'envoi au client fd=" << c->fd << std::endl;
				remove_client(i, nfds, fds);
				i--;
				continue;
			}

			if (c->send_buffer.empty()) {
				fds[i].events &= ~POLLOUT;
			}
		}
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

		tchek_listen(nfds, fds);

		tchek_clients(nfds, fds);

		tchek_clients_out(nfds, fds);

		for (int i = 0; i < nfds; ++i) {
			fds[i].revents = 0;
		}
	}
}