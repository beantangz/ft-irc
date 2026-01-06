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

Channel* Server::get_channel(const std::string &name)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->name == name)
			return (channels[i]);
	}
	return NULL;
}

void Server::command_JOIN(Client *c, std::string channel_name, int index, struct pollfd *fds, int nfds)
{

	Channel* ch = get_channel(channel_name);

	// std::cout << "ahahahahah" << std::endl;
	// if (ch && ch->has_client(c))
	// return;
	if (!ch)
	{
		ch = new Channel(channel_name);
		channels.push_back(ch);
	}
	if ( ch && ch->isInviteOnly() && !ch->isInvited(c)) 
	{
		std::cout << "sperm" << std::endl;
		send_numeric(c, "ft_irc", 473, c->nick, channel_name + " :Cannot join channel (+i)", fds, index);
		return;
	}
	std::cout << "avant addclient" << std::endl;
	if (!ch->has_client(c))
		ch->add_client(c);
	// c->channels.push_back(ch);

	//si le client est invite on le tej de la liste des invite !!! ahah
	if (ch->isInvited(c))
	{
		ch->invited_clients.erase(
			std::remove(ch->invited_clients.begin(), ch->invited_clients.end(), c),
			ch->invited_clients.end()
		);
	}
	std::string prefix = ":" + c->nick + "!" + c->user + "@" + c->host;
	std::string join_msg = prefix + " JOIN :" + channel_name + "\r\n";
	//c->queue_send(join_msg, fds, index);
	ch->broadcast(c, join_msg, fds, index, nfds);
}



void Server::command_PRIVMSG(Client *c, std::string &target, std::string &msg, struct pollfd *fds, int index, int nfds){
	std::string prefix = ":" + c->nick + "!" + c->user + "@" + c->host;

	if (target[0] == '#') 
	{
		Channel* ch = NULL;
		for (size_t i = 0; i < channels.size(); ++i)
		{
			if (channels[i]->name == target)
			{
				ch = channels[i];
				break;
			}
		}
		if (!ch) {
			numeric_403(c, target, fds, index); // No such channel
			return;
		}
		if (!ch->has_client(c))  // on verif si le client est dans le channel pour voir sil peut louvrir
		{
			send_numeric(c, "ft_irc", 442, c->nick,
				target + " :You're not on that channel",
				fds, index);
			return;
		}
		ch->broadcast(c, prefix + " PRIVMSG " + target + " :" + msg + "\r\n", fds, index, nfds);
	} 
	else 
	{
		Client* dest = find_client_by_nick(target);
		if (!dest) {
			numeric_401(c, target, fds, index);
			return;
		}
		dest->queue_send(prefix + " PRIVMSG " + target + " :" + msg + "\r\n", fds, index);
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
	if (!c->pass_ok)
		return;

	if (c->authenticated) 
		return;

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
}

void Server::command_KICK(Client* kicker, const std::string& channel_name,
						  const std::string& target_nick, const std::string& reason,
						  struct pollfd* fds, int index, int nfds)
{
	Channel* ch = find_channel(channel_name);
	if (!ch)
	{
		numeric_403(kicker, channel_name, fds, index);
		return;
	}
	if (!ch->isOperator(kicker))
	{
		numeric_482(kicker, channel_name, fds, index);
		return ;
	}
	Client* target = find_client_by_nick(target_nick);
	if (!target)
	{
		numeric_401(kicker, target_nick, fds, index); // No such nick
		return;
	}
	if (!ch->has_client(kicker))
	{
		numeric_442(kicker, channel_name, fds, index); // You're not on that channel
		return;
	}
	if (!ch->has_client(target))
	{
		numeric_441(kicker, target_nick, channel_name, fds, index);
		return;
	}
	std::string prefix = ":" + kicker->nick + "!" + kicker->user + "@" + kicker->host;
	std::string kick_msg = prefix + " KICK " + channel_name + " " + target_nick;

	if (!reason.empty())
		kick_msg += " :" + reason;
	kick_msg += "\r\n";
	ch->broadcast(NULL, kick_msg, fds, index, nfds);
	ch->remove_client(target);
}

void Server::command_INVITE(Client* inviter, const std::string& target_nick,
	const std::string& channel_name, struct pollfd* fds, int index)
{
	Channel* ch = find_channel(channel_name);
	  if (!ch)
	{
		numeric_403(inviter, channel_name, fds, index);
		return;
	}
	Client* target = find_client_by_nick(target_nick);
	if (!target)
	{
		numeric_401(inviter, target_nick, fds, index);
		return;
	}
	//Si le mec invite nest pas sur le channel de base
	if (!ch->has_client(inviter))
	{
		numeric_442(inviter, channel_name, fds, index);
		return;
	}
	if (ch->isInviteOnly())
		ch->addInvitation(target);
	std::string prefix = ":" + inviter->nick + "!" + inviter->user + "@" + inviter->host;
	std::string invite_msg = prefix + " INVITE " + target->nick + " :" + channel_name + "\r\n";
	target->queue_send(invite_msg, fds, index);
}
void Server::command_TOPIC(Client* c, const std::string& channel_name,
						   const std::string& new_topic,struct pollfd* fds, int index, int nfds)
{
	Channel* ch = find_channel(channel_name);
	if (!ch)
	{
		numeric_403(c, channel_name, fds, index);
		return;
	}
	if (!ch->has_client(c))
	{
		numeric_442(c, channel_name, fds, index);
		return;
	}
	if (new_topic.empty())
	{
		// Le client veut juste voir le topic
		if (ch->topic.empty())
			numeric_331(c, channel_name, fds, index); // Pas de topic
		else
			numeric_332(c, channel_name, ch->topic, fds, index); // Topic actuel
		return;
	}
	if (ch->isTopicProtected() && !ch->isOperator(c))
	 {
		numeric_482(c, channel_name, fds, index); // Pas le droit de changer le topic
		return;
	}
	ch->topic = new_topic;
	std::string prefix = ":" + c->nick + "!" + c->user + "@" + c->host;
	std::string topic_msg = prefix + " TOPIC " + channel_name + " :" + new_topic;
	ch->broadcast(NULL, topic_msg, fds, index, nfds);
}

void Server::handleCommand(Client* c,std::string& line, int index, struct pollfd *fds, int &nfds)
{
	std::istringstream iss(line);
	std::string cmd;
	iss >> cmd;

	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	std::cout << line << std::endl << std::flush;

	if (cmd == "PASS") {
		std::string pass;
		iss >> pass;
		if (c->authenticated) {
			numeric_462(c, fds, index);
			return;
		}
		if (pass.empty()) {
			numeric_461(c, cmd, fds, index);
			return;
		}
		if (pass == password) {
			c->pass_ok = true;
		} else {
			numeric_464(c, fds, index);
			remove_client(index, nfds, fds);
			return;
		}
	}
	else if (cmd == "NICK") {
		std::string nickname;
		iss >> nickname;
		command_NICK(c, nickname, fds, index);
		if (!c->user.empty()) {
			register_client(c, fds, index);
		}
	}
	else if (cmd == "USER") {
		std::string username, unused, mode, realname;
		iss >> username >> unused >> mode;
		std::getline(iss, realname);

		if (username.empty()) {
			numeric_461(c, cmd, fds, index);
			return;
		}
		c->user = username;
		if (!c->nick.empty()) {
		register_client(c, fds, index);
		}
	}
	else if (cmd == "JOIN")
	{
		if (!channels.empty() && channels[0])
		{
			std::cout << channels[0]->invite_only << std::endl;
		}
		if (!c->authenticated) 
		{
			numeric_451(c, fds, index);
			return;
		}
		std::string channel_name;
		iss >> channel_name;
		if (channel_name.empty())
		{
			numeric_461(c, cmd, fds, index);
			return;
		}
		command_JOIN(c, channel_name,index, fds, nfds);
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
	else if (cmd == "PRIVMSG") {
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
		command_PRIVMSG(c, target, msg, fds, index, nfds);
	}
	else if (cmd == "PING")
{
	std::string token;
	iss >> token;

	// Toujours répondre PONG d'abord
	if (!token.empty())
		c->queue_send("PONG :" + token + "\r\n", fds, index);

	// DEBUG des channels existants
	for (size_t i = 0; i < channels.size(); ++i)
		channels[i]->debug_print();
}

	else if (cmd == "KICK")
	{
		if (!c->pass_ok && !password.empty())
		{
			numeric_464(c, fds, index);
			return ;
		}
		if (!c->authenticated)
		{
			numeric_451(c, fds, index);
			return ;
		}
		std::string channel_name;
		std::string target_nick;
		std::string reason;
		iss >> channel_name >> target_nick;
		std::getline(iss, reason);
		if (!reason.empty() && reason[0] == ' ') //enlever lespace initial 	
			reason.erase(0, 1);					//qui reste au debut de reason
		if (channel_name.empty() || target_nick.empty())
		{
			numeric_461(c, cmd, fds, index);
			return ;
		}
		command_KICK(c, channel_name, target_nick, reason, fds, index, nfds);
	}
	else if (cmd == "INVITE")
	{
		if (!c->pass_ok && !password.empty())
		{
			numeric_464(c, fds, index);
			return;
		}
		if (!c->authenticated)
		{
			numeric_451(c, fds, index);
			return;
		}
		std::string target_nick;
		std::string channel_name;
		iss >> target_nick >> channel_name;
		if (target_nick.empty() || channel_name.empty())
		{
			numeric_461(c, cmd, fds, index);
			return;
		}
		command_INVITE(c, target_nick, channel_name, fds, index);
	}
	else if (cmd == "TOPIC")
	{
		if (!c->pass_ok && !password.empty())
		{
			numeric_464(c, fds, index); 
			return;
		}
		if (!c->authenticated)
		{
			numeric_451(c, fds, index);
			return;
		}
		std::string channel_name;
		std::string new_topic;
		iss >> channel_name;
		std::getline(iss, new_topic);
		if (!new_topic.empty() && new_topic[0] == ' ')
			new_topic.erase(0, 1);

		if (channel_name.empty())
		{
			numeric_461(c, cmd, fds, index);
			return;
		}
		command_TOPIC(c, channel_name, new_topic, fds, index, nfds);
	}
	else 
		numeric_421(c, cmd, fds, index);
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
					char ip_str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

					std::cout << "Nouveau client connecté: fd=" << client_fd
					<< ", IP=" << ip_str << std::endl;

				Client *c = new Client(client_fd, ip_str);
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