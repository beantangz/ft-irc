#include "irc.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

#include "Server.hpp"
#include "errors.hpp"

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
	if (name.empty() || name[0] != '#')
		return NULL;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->name == name)
			return channels[i];
	}
	Channel *newCh = new Channel(name);
	channels.push_back(newCh);
	return newCh;
}

Client* Server::find_client_by_fd(int fd) {
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->fd == fd)
			return clients[i];
	}
	return NULL;
}

std::string lower_nick(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

Client* Server::find_client_by_nick(const std::string &_nick){
	std::string target = lower_nick(_nick);
	for (size_t i = 0; i < clients.size(); ++i) {
	   if (lower_nick(clients[i]->nick) == target)
		  return clients[i];
	}
	return NULL;
}

void Server::command_NICK(Client *c, std::string &nickname, struct pollfd *fds, int index) {
	if (nickname.empty()) {
		numeric_431(c, fds, index);
		return;
	}

	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i] != c && clients[i]->nick == nickname) {
			numeric_433(c, nickname, fds, index);
			return;
		}
	}

	c->nick = nickname;
}


void Server::command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds){
		Channel* ch = find_channel(channel_name);
		if (!ch) {
			send_numeric(c, "ft_irc", 476, c->nick, "Bad channel mask", fds, index);
		return;
		}
		ch->add_client(c);
		c->channels.push_back(ch);

		std::string join_msg = ":" + c->nick + " JOIN :" + channel_name + "\r\n";
		ch->broadcast(c, join_msg, fds, index);
}

void Server::command_PRIVMSG(Client *c, std::string &target, std::string &msg, struct pollfd *fds, int index){
	if (target[0] == '#') {
		Channel* ch = NULL;
	for (size_t i = 0; i < channels.size(); ++i) {
	if (channels[i]->name == target) {
		ch = channels[i];
		break;
	}
}
		if (!ch) {
			numeric_403(c, target, fds, index); // No such channel
			return;
		}
		ch->broadcast(c, ":" + c->nick + " PRIVMSG " + target + " :" + msg + "\r\n", fds, index);
	} else {
		Client* dest = find_client_by_nick(target);
		if (!dest) {
			numeric_401(c, target, fds, index);
			return;
		}
		dest->queue_send(":" + c->nick + " PRIVMSG " + target + " :" + msg + "\r\n", fds, index);
	}
}

void Server::handleBuffer(Client* c, int index, struct pollfd *fds, int &nfds) {
	size_t pos;

	while ((pos = c->recv_buffer.find("\r\n")) != std::string::npos) {
		std::string line = c->recv_buffer.substr(0, pos);
		c->recv_buffer.erase(0, pos + 2);

		if (line.empty())
			continue;

		handleCommand(c, line, index, fds, nfds);
	}
}

void Server::register_client(Client *c, struct pollfd *fds, int index) {
	c->authenticated = true;

	c->queue_send(":ft_irc 001 " + c->nick +
				  " :Welcome to the ft_irc network, " + c->nick + "\r\n",
				  fds, index);

	c->queue_send(":ft_irc 002 " + c->nick +
				  " :Your host is ft_irc\r\n",
				  fds, index);

	c->queue_send(":ft_irc 003 " + c->nick +
				  " :This server was created today\r\n",
				  fds, index);

	c->queue_send(":ft_irc 004 " + c->nick +
				  " ft_irc 1.0 iowghraAbck\r\n",
				  fds, index);
}

void Server::handleCommand(Client* c,std::string& line, int index, struct pollfd *fds, int &nfds)
{
	std::istringstream iss(line);
	std::string cmd;
	iss >> cmd;

	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	if (cmd == "PASS") {
		std::string pass;
		iss >> pass;
		if (pass.empty()) {
			numeric_461(c, cmd, fds, index);
			return;
		}
		if (pass == password) {
			c->pass_ok = true;
		} else {
			numeric_464(c, fds, index);
			remove_client(index, nfds, fds);
		}
	}

	if (cmd == "NICK") {
		if (!c->pass_ok && !password.empty()) {
			numeric_464(c, fds, index);
			return;
		}
			std::string nickname;
		iss >> nickname;
		command_NICK(c, nickname, fds, index);
		if (!c->user.empty() && !c->authenticated) {
			register_client(c, fds, index);
}
	}
	else if (cmd == "USER") {
		if (!c->pass_ok && !password.empty()) {
			numeric_464(c, fds, index);
			return;
		}
	std::string username, unused, mode, realname;
	iss >> username >> unused >> mode;
	std::getline(iss, realname);

	if (username.empty()) {
		numeric_461(c, cmd, fds, index);
		return;
	}
	c->user = username;
	if (!c->nick.empty() && !c->authenticated) {
		register_client(c, fds, index);
	}
}
	else if (cmd == "JOIN") {
		if (!c->pass_ok && !password.empty()) {
			numeric_464(c, fds, index);
			return;
		}
		if (!c->authenticated) {
			numeric_451(c, fds, index);
			return;
			}
		std::string channel_name;
		iss >> channel_name;
		if (channel_name.empty()) {
		numeric_461(c, cmd, fds, index);
		return;
	}
		command_JOIN(c, channel_name,index, fds);
	}
	else if (cmd == "MODE")
	{
		if (!c->pass_ok && !password.empty()) {
			numeric_464(c, fds, index);
			return;
		}
		std::string target;
		std::string modes;
		std::string param;

		iss >> target >> modes;
		if (!iss.eof())
			iss >> param;
		command_MODE(c, target, modes, param, index, fds);
	}
	else if (cmd == "PRIVMSG") {
		if (!c->pass_ok && !password.empty()) {
			numeric_464(c, fds, index);
			return;
		}
	if (!c->authenticated) {
		numeric_451(c, fds, index);
		return;
	}
	std::string target;
	iss >> target;
	if (target.empty()) {
		numeric_461(c, cmd, fds, index);
		return;
	}
	std::string msg;
	std::getline(iss, msg);
	if (!msg.empty() && msg[0] == ' ')
		msg.erase(0, 1); // enlever l'espace avant le message
	if (msg.empty()) {
		numeric_412(c, fds, index);
		return;
	}
	command_PRIVMSG(c, target, msg, fds, index);
}
	else {
		numeric_421(c, cmd, fds, index);
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

void Server::remove_client(int index, int &nfds, struct pollfd *fds) {
	Client *c = find_client_by_fd(fds[index].fd);
	if (!c) return;

	std::cout << "Client déconnecté: fd=" << c->fd << std::endl;
	close(c->fd);

	for (size_t i = 0; i < channels.size(); i++){
		channels[i]->remove_client(c);
	}
	clients.erase(std::remove(clients.begin(), clients.end(), c), clients.end());

	c->recv_buffer.clear();
	c->send_buffer.clear();
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
				handleBuffer(c, i, fds, nfds);
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