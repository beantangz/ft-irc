#include "irc.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

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

Channel* Server::find_or_create_channel(const std::string &name) {
	for (Channel *ch : channels) {
		if (ch->name == name)
			return ch;
	}
	Channel *newCh = new Channel(name);
	channels.push_back(newCh);
	return newCh;
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
					
					clients.erase(std::find(clients.begin(), clients.end(), c));
					delete c;
					i--;
					continue;
				}
				 else {
					c->buffer.append(buf, bytes);

					// TODO : parser les lignes complètes terminées par \r\n
					// et exécuter commandes IRC (NICK, USER, JOIN, PRIVMSG...)
				}
			}
		}
	}
}