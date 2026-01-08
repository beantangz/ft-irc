#include "Server.hpp"
#include "Channel.hpp"
#include "errors.hpp"
#include <iostream>
//target = nom du channel




//pour le +k
void Server::send_numeric_475(Client* c, const std::string& server_name,
                              const std::string& channel, struct pollfd* fds, int index)
{
    std::ostringstream oss;
    oss << ":" << server_name << " 475 " 
        << c->nick << " " 
        << channel << " "
        << ":Cannot join channel (+k)\r\n";
    c->queue_send(oss.str(), fds, index);
}

Channel* Server::check_error_mode(Client *c, const std::string &target,struct pollfd *fds,
								  int index)
{
	if (target.empty() || target[0] != '#')
	{
		if (target[0] != '#'){
			if (find_client_by_nick(target))
				return NULL;
			numeric_401(c, target, fds, index);
			return NULL;
		}
		numeric_403(c, target, fds, index);
		return NULL;
	}

	Channel *ch = find_channel(target);
	if (!ch)
	{
		numeric_403(c, target, fds, index);
		return NULL;
	}

	if (!ch->isOperator(c))
	{
		numeric_482(c, target, fds, index);
		return NULL;
	}

	return ch;
}

void Server::mode_operator(Client *c, Channel *ch, char sign, const std::string &param, int index, struct pollfd *fds)
 {
	if (param.empty())
	{
		numeric_461(c, "MODE", fds, index);
		return;
	}
	Client *target_client = find_client_by_nick(param);
	if (!target_client)
	{
		numeric_401(c, param, fds, index);
		return;
	}
	if (sign == '+')
		ch->addOperator(target_client);
	else if(sign == '-')
		ch->removeOperator(target_client);
	else
	{
		 numeric_472(c, "o", fds, index);
		 return ;
	}
	std::string prefix = ":" + c->nick + "!" + c->user + "@" + c->host;
	std::string msg = prefix + " MODE " + ch->name + " " + sign + "o " + param + "\r\n";
	// notifier tous les clients
	for (size_t i = 0; i < ch->clients.size(); i++)
		ch->clients[i]->queue_send(msg, fds, index);
 }

void Server::mode_invite_only(Client *c, Channel *ch, char sign, int index, struct pollfd *fds, int nfds)
{
	if (sign == '+')
	{
		ch->invite_only = true;
		std::cout << "invite only = true" << std::endl;
	}
	else if (sign == '-')
	{
		ch->invite_only = false;
		std::cout << "invite only = false" << std::endl;
	}
	else
	{
		numeric_472(c, "i", fds, index);
		return;
	}
	std::string msg = ":" + c->nick + " MODE " + ch->name +
					  " " + sign + "i\r\n";
	for (size_t i = 0; i < ch->clients.size(); i++)
	{
		Client* dest = ch->clients[i];
		int idx = find_index_in_fds(dest->fd, fds, nfds);
		if (idx >= 0)
			dest->queue_send(msg, fds, idx);
	}
}

void Server::mode_topic_only(Client *c, Channel *ch, char sign, int index, struct pollfd *fds) 
{
	if (sign == '+')
		ch->topic_op_only = true;
	else if (sign == '-')
		ch->topic_op_only = false;
	else
	{
		numeric_472(c, "t", fds, index);
		return;
	}
	std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "t\r\n";
	for (size_t i = 0; i < ch->clients.size(); i++)
		ch->clients[i]->queue_send(msg, fds, index);
}

void Server::mode_key(Client *c, Channel *ch, char sign,
					  const std::string &param, int index, struct pollfd *fds, int nfds)
{
	if (sign == '+')
	{
		if (param.empty())
		{
			numeric_461(c, "MODE", fds, index);
			return;
		}
		ch->has_key = true;
		ch->key = param;
	}
	else if (sign == '-')
	{
		ch->has_key = false;
		ch->key.clear();
	}
	else
	{
		numeric_472(c, "k", fds, index);
		return ;
	}
	std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "k";
	if (sign == '+')
    	msg += " " + ch->key;
	msg += "\r\n";
	for (size_t i = 0; i < ch->clients.size(); i++)
	{
    	Client* dest = ch->clients[i];
    	int idx = find_index_in_fds(dest->fd, fds, nfds);
    if (idx >= 0)
        dest->queue_send(msg, fds, idx);
	}
}

void Server::mode_limit(Client *c, Channel *ch, char sign,
						const std::string &param, int index, struct pollfd *fds)
{
	if (sign == '+')
	{
		if(param.empty())
		{ 
			numeric_461(c, "MODE", fds, index);
			return;
		}
		int limit = 0;
		std::istringstream iss(param);
		if (!(iss >> limit) || limit <= 0)
		{
			numeric_461(c, "MODE", fds, index);
			return ;
		}
		ch->has_limit = true;
		ch->user_limit = limit;
	}
	else if (sign == '-')
	{
		ch->has_limit = false;
		ch->user_limit = 0;
	}
	else
	{
		numeric_472(c, "l", fds, index); // mode inconnu
		return;
	}
	 std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "l";
	if (sign == '+')
		msg += " " + param;
	msg += "\r\n";
	for (size_t i = 0; i < ch->clients.size(); i++)
		ch->clients[i]->queue_send(msg, fds, index);
}


void Server::command_MODE(Client *c, const std::string target, std::string mode, std::string param,
						  int index, struct pollfd *fds, int nfds)
{
	Channel *ch = check_error_mode(c, target, fds, index);
	if (!ch)
		return;
	if (mode.length() < 2)
	{
		numeric_472(c, mode, fds, index);
		return;
	}
	char sign = mode[0];
	char m = mode[1];
	if (m == 'o')
		mode_operator(c, ch, sign, param, index, fds);
	else if (m == 'i')
		mode_invite_only(c, ch, sign, index, fds, nfds);
	else if (m == 't')
		mode_topic_only(c, ch, sign, index, fds);
	else if (m == 'k')
		mode_key(c, ch, sign, param, index, fds, nfds);
	else if (m == 'l')
		mode_limit(c, ch, sign, param, index, fds);
	else
		numeric_472(c, std::string(1, m), fds, index);
}