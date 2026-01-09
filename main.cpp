#include "irc.hpp"
#include <iostream>
#include <signal.h>

volatile sig_atomic_t g_running = 1;

void sigint_handler(int)
{
	g_running = 0;
}

int main(int ac, char **av)
{
	if (ac != 3 && ac != 2)
	{
		std::cout << "invalid format\n";
		return 0;
	}
	signal(SIGINT, sigint_handler);

	Server serv(std::atoi(av[1]), av[2]);
	serv.run();

	return 0;
}