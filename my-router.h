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

typedef std::unordered_map<char, int> mapc_int;
typedef std::unordered_map<char, routing_info> mapc_rt;
typedef std::vector<std::string> vstring;
typedef std::vector<char> vchar;

class router
{
	public:
		router(char srv_name, char* tp_file);
		~router();
		void test();
		void run_router();

	private:
		mapc_int dv; //distance std::vector: map dest name to cost
		mapc_rt rt; //routing table: map dest name to struct of info
		vchar nb; //neighbors
		int s; //socket

		char server_name;
		unsigned short server_port;

		void initialize(char* tp_file);
		std::string form_msg();
		mapc_int parse_msg(std::string msg);
		void handle_msg(char* t_msg, int msg_len);
		void handle_dv_update(std::string msg, char sender_name);
		void update_rt(char sender_name, mapc_int* rcvd_dv);
		bool update_dv();
		unsigned short name_to_port(char name);

};

#endif