#ifndef ROUTER_H
#define ROUTER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

typedef struct
{
	int cost;
	unsigned short out_port;
	unsigned short dest_port;
	char next_hop;
} routing_info;

//typedef std::unordered_map<char, int> mapc_int;
//typedef std::unordered_map<char, routing_info> mapc_rt;
//typedef std::vector<char> vchar;
//typedef std::vector<std::string> vstring;

class router
{
	public:
		router(char srv_name, char* tp_file);
		~router();
		void test();
		void run_router();

	private:
		std::unordered_map<char,int > dv; //distance std::vector: map dest name to cost
		std::unordered_map<char, routing_info > rt; //routing table: map dest name to struct of info
		std::vector<char > nb; //neighbors
		std::unordered_map<char, int > link_costs;
		int s; //socket
		std::string file; //topology file

		//for dropped routers
		std::vector<char> dr; //dropped routers
		std::unordered_map<char, int> updates; //for use in checking if router is dead

		char server_name;
		unsigned short server_port;

		void initialize();
		std::string form_msg(char type); //U for DV update, D for death
		std::unordered_map<char, int> parse_msg(std::string msg);
		void handle_msg(char* t_msg, int msg_len);
		void handle_dv_update(std::string msg, char sender_name);
		bool update_rt(char sender_name, std::unordered_map<char, int>* rcvd_dv);
		bool update_dv();
		unsigned short name_to_port(char name);
		void handle_forward_msg(std::string msg, char sender_name);
		void broadcast(char type);
		void print_dv(std::unordered_map<char, int>* rdv);
		void print_rt(std::unordered_map<char, routing_info>* rrt);
		void check_dead();
		void handle_death_msg(std::string msg, char sender_name);
};

#endif