#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define MAX_CLIENTS 1024

class Server 
{
public:
	int					 listen_fd;
	std::vector<Client*>	clients;
	std::vector<Channel*>   channels;
	std::string			 password;

	Server(int port, const std::string &password = "");
	~Server();

	void	run();
	void	tchek_listen(int &nfds, struct pollfd *fds);
	void	tchek_clients(int &nfds, struct pollfd *fds);
	void	tchek_clients_out(int nfds, struct pollfd *fds);


	void	handleBuffer(Client *c, int index, struct pollfd *fds, int &nfds);
	void	handleCommand(Client *c, std::string &line, int index, struct pollfd *fds, int &nfds);

	void	remove_client(int index, int &nfds, struct pollfd *fds);
	void	register_client(Client *c, struct pollfd *fds, int index);


	Channel* find_channel(const std::string &name);

	Client* find_client_by_fd(int fd);
	Client* find_client_by_nick(const std::string &nick);
	void send_numeric_473(Client* c,
                              const std::string& channel,
                              struct pollfd* fds,
                              int index);
	// MODE fonctions
	Channel* check_error_mode(Client *c, const std::string &target, struct pollfd *fds, int index);

	void	mode_operator(Client *c, Channel *ch, char sign,
						  const std::string &param, int index, struct pollfd *fds);
	void	mode_invite_only(Client *c, Channel *ch, char sign,
							 int index, struct pollfd *fds, int nfds);
	void	mode_topic_only(Client *c, Channel *ch, char sign,
							int index, struct pollfd *fds);
	void	mode_key(Client *c, Channel *ch, char sign,
					 const std::string &param, int index, struct pollfd *fds, int nfds);
	void	mode_limit(Client *c, Channel *ch, char sign,
					   const std::string &param, int index, struct pollfd *fds);
	Channel* get_channel(const std::string &name);

	std::string get_join_key(const std::string &line, const std::string &channel_name);

	void send_numeric_475(Client* c, const std::string& server_name,
                              const std::string& channel, struct pollfd* fds, int index);
	//commmands
	void command_TOPIC(Client* c, const std::string& channel_name,
                           const std::string& new_topic,struct pollfd* fds, int index, int nfds);
	void command_KICK(Client* kicker, const std::string& channel_name,
                          const std::string& target_nick, const std::string& reason,
                          struct pollfd* fds, int index, int nfds);
	void command_INVITE(Client* inviter, const std::string& target_nick,
	const std::string& channel_name, struct pollfd* fds, int index, int nfds);
	void command_MODE(Client *c, std::string target, std::string mode,
		 std::string param, int index, struct pollfd *fds, int nfds);
	void command_NICK(Client *c, std::string &nickname, struct pollfd *fds, int index);
	void command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds, int nfds, const std::string &full_line, const std::string &key_from_user);
	void command_PRIVMSG(Client *c, std::string &target, std::string &msg, struct pollfd *fds, int index, int nfds);
};
