#pragma once

#include "irc.hpp"

class Server {
public:
	int					 listen_fd;
	std::vector<Client*>	clients;
	std::vector<Channel*>   channels;
	std::string			 password;

	Server(int port, const std::string &password = ""); //pourquoi??
	~Server();

	void init_socket();
	void run();
	void handleBuffer(Client *c);
	void handleCommand(Client *c, std::string& line);
	void accept_new_client();
	void handle_client(Client *c);
	Channel* find_channel(const std::string &name);

	//commmands
	void command_NICK(Client *c, std::string nickname);
	void command_JOIN(Client *c, std::string channel_name);
};
