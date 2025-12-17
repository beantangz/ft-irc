#include "Server.hpp"
#include "Channel.hpp"

void Server::command_MODE(Client *c, const std::string target, std::string mode, std::string param, int index, struct pollfd *fds)
{
	if (target[0] == '#')
	{
		Channel* ch = find_channel(target);
	}
}