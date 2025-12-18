#include "Server.hpp"
#include "Channel.hpp"

Channel* Server::check_error_mode(Client *c,
                                  const std::string &target)
{
    if (target.empty() || target[0] != '#')
    {
        send_numeric(c, "ft_irc", 403, target, "No such channel");
        return NULL;
    }

    Channel *ch = find_channel(target);
    if (!ch)
    {
        send_numeric(c, "ft_irc", 403, target, "No such channel");
        return NULL;
    }

    if (!ch->isOperator(c))
    {
        send_numeric(c, "ft_irc", 482, target, "You're not channel operator");
        return NULL;
    }

    return ch;
}
 
							

void Server::command_MODE(Client *c, const std::string target, std::string mode, std::string param, int index, struct pollfd *fds)
{
   Channel *ch = check_error_mode(c, target);
    if (!ch)
        return;
	
}