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

	void handleBuffer(Client *c, int index, struct pollfd *fds);
	void handleCommand(Client *c, std::string& line, int index, struct pollfd *fds);

	Client* find_client_by_fd(int fd);
	void remove_client(int index, int &nfds, struct pollfd *fds);


	void	register_client(Client *c, struct pollfd *fds, int index);
	Channel* find_channel(const std::string &name);
	Channel* Server::check_error_mode(Client *c, const std::string &target);

	//commmands
	void command_MODE(Client *c, std::string target, std::string mode,
		 std::string param, int index, struct pollfd *fds);
	void command_NICK(Client *c, std::string &nickname);
	void command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds);
};
