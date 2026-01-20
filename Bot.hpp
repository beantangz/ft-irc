#pragma once

#include "Client.hpp"

class Bot : public Client {

	public :
	Bot(const std::string &nick_, const std::string &user_, const std::string &host_ip);
	bool connect_to_server(const std::string &ip, int port);
	void send_message(const std::string &msg);
	void handle_message(const std::string &msg);
};