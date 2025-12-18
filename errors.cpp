#include "irc.hpp"

void send_numeric(Client* c, const std::string& server_name, int code,
				  const std::string& target, const std::string& message, struct pollfd *fds, int index) {

	std::ostringstream oss;
	oss << ":" << server_name << " "
		<< code << " "
		<< target << " :"
		<< message << "\r\n";

	c->queue_send(oss.str(), fds, index);
}



void numeric_001(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 001, c->nick, "Welcome to ft_irc!", fds, index);
}

void numeric_002(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 002, c->nick,
		"Your host is ft_irc",
		fds, index);
}

void numeric_003(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 003, c->nick,
		"This server was created today",
		fds, index);
}

void numeric_004(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 004, c->nick,
		"ft_irc 1.0 oiwtkl",
		fds, index);
}

void numeric_401(Client* c, const std::string& target, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 401, c->nick,
		target + " :No such nick/channel",
		fds, index);
}

void numeric_412(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 412, c->nick,
		"No text to send",
		fds, index);
}

void numeric_403(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 403, c->nick,
		channel + " :No such channel",
		fds, index);
}

void numeric_421(Client* c, const std::string& cmd, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 421, c->nick,
		cmd + " :Unknown command",
		fds, index);
}
//kick
void numeric_441(Client* c, const std::string& nick, const std::string& channel,
				 struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 441, c->nick,
		nick + " " + channel + " :They aren't on that channel",
		fds, index);
}

void numeric_442(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 442, c->nick,
		channel + " :You're not on that channel",
		fds, index);
}

void numeric_451(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 451, "*",
		"You have not registered",
		fds, index);
}

void numeric_461(Client* c, const std::string& cmd, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 461, c->nick,
		cmd + " :Not enough parameters",
		fds, index);
}

void numeric_431(Client* c, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 431, "*", "No nickname given", fds, index);
}

void numeric_433(Client* c, const std::string& nick, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 433, nick, "Nickname is already in use", fds, index);
}

void numeric_472(Client* c, const std::string& mode, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 472, c->nick,
		mode + " :is unknown mode char",
		fds, index);
}

void numeric_473(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 473, c->nick,
		channel + " :Cannot join channel (+i)",
		fds, index);
}

void numeric_474(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 474, c->nick,
		channel + " :Banned from channel",
		fds, index);
}

void numeric_475(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 475, c->nick,
		channel + " :Cannot join channel (+k)",
		fds, index);
}

void numeric_482(Client* c, const std::string& channel, struct pollfd *fds, int index) {
	send_numeric(c, "ft_irc", 482, c->nick,
		channel + " :You're not channel operator",
		fds, index);
}
