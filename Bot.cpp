#include "Bot.hpp"
#include <arpa/inet.h>
#include <iostream>

Bot::Bot(const std::string &nick_, const std::string &user_, const std::string &host_ip) : Client(-1, host_ip)
{
	nick = nick_;
	user = user_;
	authenticated = true;
	pass_ok = true;
}

bool Bot::connect_to_server(const std::string &ip, int port){
	fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd < 0)
		return false;
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);

	if (::connect(fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		close(fd);
		fd = -1;
		return false;
	}
	std::cout << "Bot connecté au serveur" << std::endl << std::flush;
	return true;
}

void Bot::send_message(const std::string &msg) {
	if (fd != -1)
		send(fd, msg.c_str(), msg.size(), 0);
}

void Bot::handle_message(const std::string &msg) {
	if (msg.find("PING") != std::string::npos) 
	{
		std::string response = "PONG ft_irc\r\n";
		send_message(response);
	}
}

void bot_loop(Bot &bot) {
	char buffer[512];
	while (true) {
		int bytes = recv(bot.fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes <= 0) break;
		buffer[bytes] = '\0';
		std::string msg(buffer);
		std::cout << "Reçu : " << msg;

		bot.handle_message(msg); // répondre au PING
	}
}