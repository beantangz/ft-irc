#include "Server.hpp"
#include "Channel.hpp"
#include "errors.hpp"

//target = nom du channel
Channel* Server::check_error_mode(Client *c, const std::string &target,struct pollfd *fds,
                                  int index)
{
    if (target.empty() || target[0] != '#')
    {
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

Client* Server::find_client_by_nick(const std::string &nick)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i]->nick == nick)
            return clients[i];
    }
    return NULL;
}


 void Server::mode_operator(Client *c, Channel *ch, char sign, const std::string &param, int index, struct pollfd *fds)
 {
	if (param.empty())
	{
		send_numeric(c, "ft_irc", 461, "MODE", "Not enough parameters");
		return;
	}
	Client *target_client = find_client_by_nick(param);
	if (!target_client)
	{
		send_numeric(c, "ft_irc", 401, param, "No such Nick");  //a changer
		return;
	}
	if (sign == '+')
		ch->addOperator(target_client);
	else
		ch->removeOperator(target_client);
	
    std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "o " + param + "\r\n";
    // notifier tous les clients
    for (size_t i = 0; i < ch->clients.size(); i++)
        ch->clients[i]->queue_send(msg, fds, index);
 }

 void Server::mode_invite_only(Client *c, Channel *ch, char sign, int index, struct pollfd *fds)
 {
	if (sign == '+')
		ch->invite_only = true;
	std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "i\r\n";
    for (size_t i = 0; i < ch->clients.size(); i++)
        ch->clients[i]->queue_send(msg, fds, index);
 }

void Server::mode_topic_only(Client *c, Channel *ch, char sign, int index, struct pollfd *fds) 
{
    if (sign == '+')
		ch->topic_op_only = true;
    std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "t\r\n";
    for (size_t i = 0; i < ch->clients.size(); i++)
        ch->clients[i]->queue_send(msg, fds, index);
}

void Server::mode_key(Client *c, Channel *ch, char sign,
                      const std::string &param, int index, struct pollfd *fds)
{
    if (sign == '+')
    {
        if (param.empty())
        {
            send_numeric(c, "ft_irc", 461, "MODE", "Not enough parameters");
            return;
        }
        ch->has_key = true;
        ch->key = param;
    }
    else
    {
        ch->has_key = false;
        ch->key.clear();
    }
    std::string msg = ":" + c->nick + " MODE " + ch->name + " " + sign + "k\r\n";
    for (size_t i = 0; i < ch->clients.size(); i++)
        ch->clients[i]->queue_send(msg, fds, index);
}


void Server::command_MODE(Client *c, const std::string target, std::string mode, std::string param, int index, struct pollfd *fds)
{
   Channel *ch = check_error_mode(c, target, fds);
    if (!ch)
        return;
	if (mode.length() < 2)
	{
		send_numeric(c, "ft_irc", 472, "MODE", "Unknownn mode");
		return ;
	}
	char sign = mode[0];
	char m = mode[1];

	if (m == 'o')
		mode_operator(c, ch, sign, param, index, fds);
}