#pragma once

#include "irc.hpp"
class Client;

class Channel {

public:
	std::string			name;

	std::vector<Client*> clients;
	std::vector<Client*> operators;

	Channel(const std::string &name_);
	~Channel();

	//MODES ===========

	
	bool invite_only; //+i
	bool topic_op_only; //+t

	bool has_key;//+k
	std::string key;

	bool has_limit;//+l
	int user_limit;

	bool has_client(Client* c);
	bool isOperator(Client *c);
	void addOperator(Client *c);
	void removeOperator(Client *c);
	void add_client(Client *c);
	void remove_client(Client *c);
	void broadcast(Client *from, const std::string &msg, 
		struct pollfd *fds, int index);
	
};

// =======MODE +o -o ======
// donne ou retire statu op a un client 
// seul un op peut donner ou retirer des droits
// premier user du channel est automatiquement op  
// verif isOperator(c) avant un MODE +o -o

//=====MODE +i -i
//Channel devient prive seuls les users invite peubent le rejoindre
//le bool invite_only devient true
// Quand un user tente de JOIN un channel +i:
//- Verif sil est dans la liste des invites invited_users(a faire)
//Sil ny est pas envoyer le messsage derreur 473 cannot join channel


//=======MODE +t -t 
//Topic operator only
//Seuls les operateurs peuvent changer le topic du channel
//changer le bool topic_only
//Quand un user veut change le topic:
//-Verfi si topic_op_only est actif
//-Verif si User est op
// Sil ne les pas et que topic_op_only est actif renvoyer mess derreur 482 Youre not channel operator


//======MODE +k -k
//Channel key (mdp)
//Channel protege par un mdp
//stockee dans bool has_key;
//si le channel a une cle (has_key) luser doit fournir un mdp
//Sil fournit un mauvais mdp Lui envoyer 475 Cannot join channel


//======MODE +l -l
//User limit
//stocke dans has_limit
//Quand on fait un join, si le channel est plein (clients.size() >- user_limit)
//et has_limit ==true
//Refusez lacces avec 471 Channel is full