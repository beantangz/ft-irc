#pragma once

#include "irc.hpp"

void send_numeric(Client* c, const std::string& server_name, int code,
				  const std::string& target, const std::string& message, struct pollfd *fds, int index);

void numeric_001(Client* c, struct pollfd *fds, int index);

void numeric_002(Client* c, struct pollfd *fds, int index);

void numeric_003(Client* c, struct pollfd *fds, int index);

void numeric_004(Client* c, struct pollfd *fds, int index);

void numeric_341(Client* c, const std::string& target_nick, const std::string& channel,
                 struct pollfd *fds, int index);
void numeric_401(Client* c, const std::string& target, struct pollfd *fds, int index);

void numeric_412(Client* c, struct pollfd *fds, int index);

void numeric_403(Client* c, const std::string& channel, struct pollfd *fds, int index);

void numeric_421(Client* c, const std::string& cmd, struct pollfd *fds, int index);

void numeric_441(Client* c, const std::string& nick, const std::string& channel,
				 struct pollfd *fds, int index);

void numeric_442(Client* c, const std::string& channel, struct pollfd *fds, int index);

void numeric_451(Client* c, struct pollfd *fds, int index);

void numeric_461(Client* c, const std::string& cmd, struct pollfd *fds, int index);

void numeric_431(Client* c, struct pollfd *fds, int index);

void numeric_433(Client* c, const std::string& nick, struct pollfd *fds, int index);

void numeric_464(Client* c, struct pollfd *fds, int index);

void numeric_472(Client* c, const std::string& mode, struct pollfd *fds, int index);

void numeric_473(Client* c, const std::string& channel, struct pollfd *fds, int index);

void numeric_474(Client* c, const std::string& channel, struct pollfd *fds, int index);

void numeric_475(Client* c, const std::string& channel, struct pollfd *fds, int index);

void numeric_482(Client* c, const std::string& channel, struct pollfd *fds, int index);