#include "irc.hpp"

void send_numeric(Client* c, const std::string& server_name, int code,
				  const std::string& target, const std::string& message) {

	std::ostringstream oss;
	oss << ":" << server_name << " "
		<< code << " "
		<< target << " :"
		<< message << "\r\n";

	c->send_data(oss.str());
}

void numeric_431(Client* c) {
	send_numeric(c, "ft_irc", 431, "*", "No nickname given");
}

void numeric_433(Client* c, const std::string& nick) {
	send_numeric(c, "ft_irc", 433, nick, "Nickname is already in use");
}

void numeric_001(Client* c) {
	send_numeric(c, "ft_irc", 001, c->nick, "Welcome to ft_irc!");
}

