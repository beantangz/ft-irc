#include "irc.hpp"
#include <iostream>

int main(int ac, char **av)
{
	if (ac != 3 && ac != 2)
	{
		std::cout << "invalid format\n";
		return 0;
	}
	Server serv(std::atoi(av[1]), av[2]);
	serv.run();
	return 0;
}