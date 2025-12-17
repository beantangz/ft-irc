#pragma once


#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"




//gestion erreurs :
void send_numeric(Client* c, const std::string& server_name, int code,
				  const std::string& target, const std::string& message);
void numeric_431(Client* c);
void numeric_433(Client* c, const std::string& nick);
void numeric_001(Client* c);
