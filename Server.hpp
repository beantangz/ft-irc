#pragma once

#include "irc.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#define MAX_CLIENTS 1024

class Server 
{
public:
    int                     listen_fd;
    std::vector<Client*>    clients;
    std::vector<Channel*>   channels;
    std::string             password;

    Server(int port, const std::string &password = "");
    ~Server();

    // === RUN / POLL ===
    void    run();
    void    tchek_listen(int &nfds, struct pollfd *fds);
    void    tchek_clients(int &nfds, struct pollfd *fds);
    void    tchek_clients_out(int nfds, struct pollfd *fds);

    // === CLIENT HANDLING ===
    void    accept_new_client();
    void    handle_client(Client *c);
    void    handleBuffer(Client *c, int index, struct pollfd *fds);
    void    handleCommand(Client *c, std::string &line, int index, struct pollfd *fds);

    void    remove_client(int index, int &nfds, struct pollfd *fds);

    // === FINDERS ===
    Client*  find_client_by_fd(int fd);
    Client*  find_client_by_nick(const std::string &nick);
    Channel* find_channel(const std::string &name);

	// MODE fonctions
    Channel* check_error_mode(Client *c, const std::string &target);
    void    mode_operator(Client *c, Channel *ch, char sign,
                          const std::string &param, int index, struct pollfd *fds);
    void    mode_invite_only(Client *c, Channel *ch, char sign,
	                         int index, struct pollfd *fds);
    void    mode_topic_only(Client *c, Channel *ch, char sign,
                            int index, struct pollfd *fds);
    void    mode_key(Client *c, Channel *ch, char sign,
                     const std::string &param, int index, struct pollfd *fds);
    void    mode_limit(Client *c, Channel *ch, char sign,
                       const std::string &param, int index, struct pollfd *fds);
    // === COMMANDS ===
    void command_MODE(Client *c, std::string target, std::string mode,
                      std::string param, int index, struct pollfd *fds);


	void	register_client(Client *c, struct pollfd *fds, int index);
	Channel* find_channel(const std::string &name);
	Channel* Server::check_error_mode(Client *c, const std::string &target, struct pollfd *fds, int index);

	//commmands
	void command_MODE(Client *c, std::string target, std::string mode,
		 std::string param, int index, struct pollfd *fds);
	void command_NICK(Client *c, std::string &nickname, struct pollfd *fds, int index);
	void command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds);
};
