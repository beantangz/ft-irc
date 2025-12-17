#pragma once

#include "irc.hpp"

#define MAX_CLIENTS 1024

class Server {
public:
	int					 listen_fd;
	std::vector<Client*>	clients;
	std::vector<Channel*>   channels;
	std::string			 password;

	Server(int port, const std::string &password = "");
	~Server();


	void	tchek_listen(int &nfds, struct pollfd *fds);
	void	tchek_clients(int &nfds, struct pollfd *fds);
	void tchek_clients_out(int nfds, struct pollfd *fds);

	void run();

	void handleBuffer(Client *c);
	void handleCommand(Client *c, std::string& line);

	Client* find_client_by_fd(int fd);
	void remove_client(int index, int &nfds, struct pollfd *fds);


	void accept_new_client();
	void handle_client(Client *c);
	Channel* find_channel(const std::string &name);

	//commmands
	void command_NICK(Client *c, std::string nickname);
	void command_JOIN(Client *c, std::string channel_name);
};
