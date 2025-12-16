#pragma once


#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <algorithm>


class Client;
class Channel;
class Server;


class Client {
public:
	int				 fd;
	std::string		 nick;
	std::string		 user;
	std::string		 buffer;
	std::vector<Channel*> channels;
	bool				authenticated;

	Client(int fd_);
	~Client();

	int recv_data();
	void send_data(const std::string &msg);
	void join_channel(Channel *ch);
	void leave_channel(Channel *ch);
};

class Channel {

public:
	std::string		 name;
	std::vector<Client*> clients;

	Channel(const std::string &name_);
	~Channel();

	void add_client(Client *c);
	void remove_client(Client *c);
	void broadcast(Client *from, const std::string &msg);
};


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
	void accept_new_client();
	void handle_client(Client *c);
	Channel* find_or_create_channel(const std::string &name);
};