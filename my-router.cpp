#include "my-router.h"
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#define BUFSIZE 1024

void router::print_dv(std::unordered_map<char, int>* rdv)
{
	for(std::unordered_map<char, int>::iterator it = (*rdv).begin(); it != (*rdv).end(); it++)
		std::cout << "Router " << it->first << ", Cost " << it->second << std::endl;
	std::cout << std::endl;
}

void router::print_rt(std::unordered_map<char, routing_info>* rrt)
{
	for(std::unordered_map<char, routing_info>::iterator it = (*rrt).begin(); it != (*rrt).end(); it++)
	{
		std::cout << "Router" << it->first << std::endl;
		std::cout << "\tCost: " << (it->second).cost << std::endl;
		std::cout << "\tOutgoing port: " << (it->second).out_port << std::endl;
		std::cout << "\tDestination port: " << (it->second).dest_port << std::endl;
		std::cout << "\tNext hop: " << (it->second).next_hop << std::endl;
	}
	std::cout << std::endl;
}

void router::test()
{

	std::cout << "DV:" << std::endl;
	print_dv(&dv);

	std::cout << "\nRouting Table:" << std::endl;
	print_rt(&rt);

	std::cout << "\nNeighbors:" << std::endl;
	for(int i=0; i<nb.size(); i++)
		std::cout << "\t" << nb[i] << std::endl;
	std::cout << std::endl;

	// std::string msg = form_msg();
	// std::cout << "DV msg format: " << msg << std::endl;

	// std::unordered_map<char, int> tdv = parse_msg(msg);
	// std::cout << "Parsed info: \n";
	// for(std::unordered_map<char, int>::iterator it = tdv.begin(); it != tdv.end(); it++)
	// 	std::cout << "Router " << it->first << ", Cost " << it->second << std::endl;
}

unsigned short router::name_to_port(char name)
{
	switch(name)
    {
    	case 'A':
			return 10000;
		case 'B':
			return 10001;
		case 'C':
			return 10002;
		case 'D':
			return 10003;
		case 'E':
			return 10004;
		case 'F':
			return 10005;
		default:
			return 0; //invalid name
    }
}

std::string router::form_msg(char type)
{
	std::string msg;
	if(type == 'U') //update
		msg+="D-"; //D at front indicates that this is a DV update
	if(type == 'D') //death
		msg+="K-"; //K at front indicates that this is a death msg
	msg+=server_name; //check where msg is coming from

	if(type == 'U')
	{
		for(std::unordered_map<char, int>::iterator it = dv.begin(); it != dv.end(); it++)
		{
			msg+='|';
			msg+=it->first;
			msg+='/'+std::to_string((long long int)it->second);	
		}
	}
	else //death msg
	{
		for(int i=0; i<dr.size(); i++)
		{
			msg+='|';
			msg+=dr[i]; //add dead router to msg
		}
	}
	return msg;
}

std::unordered_map<char, int> router::parse_msg(std::string msg)
{
	std::unordered_map<char, int> rcvd_dv; //to create dv sent over
	std::stringstream ss(msg);
	std::string pair,elem; //to store key value pairs
	bool c=true; //for switching
	char name;
	int cost;

	getline(ss,pair,'|'); //skip empty first elem

	while(getline(ss,pair,'|'))
	{
		name=true; //to be safe
		std::stringstream ps(pair);
		while(getline(ps,elem,'/'))
		{
			if(c)
				name=elem[0];
			else
				cost=stoi(elem);
			c=!c;
		}
		rcvd_dv[name]=cost;
	}

	return rcvd_dv;
}

bool router::update_rt(char sender_name, std::unordered_map<char, int>* rcvd_dv) 
{
    // Get the node information
    int costFromTable, newCost, currentCost;
    char currentNode;
    bool exists, isdead, rt_changed = false;

    int costToNode = link_costs[sender_name]; //THIS has to be link cost!!

    // Iterate over the distance vector
    std::unordered_map<char, int>::iterator it;
    for(it = (*rcvd_dv).begin(); it != (*rcvd_dv).end(); it++) 
    {
        currentNode = it->first; //sender to curr node
        currentCost = it->second; //cost from sender to curr node
        isdead = (std::find(dr.begin(),dr.end(),currentNode) != dr.end());
        if(isdead)
        	continue;

        // Check if the node exists in the routing table
        if (currentNode != server_name) 
        {
            exists = (rt.find(currentNode) != rt.end());
            if (exists) 
            {
                // Update if necessary
                costFromTable = rt[currentNode].cost;
                newCost = costToNode + currentCost;
                if (newCost < costFromTable) 
                {
                    rt[currentNode].cost = newCost;
                    rt[currentNode].next_hop = sender_name; 
                    rt_changed = true;
                }
                else if ((newCost == costFromTable) && (sender_name < rt[currentNode].next_hop)) 
                {
                    rt[currentNode].next_hop = sender_name;
                    rt_changed = true;
                }
            }
            else {
                // Add to the table
                rt[currentNode].cost = costToNode + currentCost;
                rt[currentNode].next_hop = sender_name;
                rt[currentNode].out_port = server_port;
                rt[currentNode].dest_port = name_to_port(currentNode);
                rt_changed = true;
            }
        }
    }
    return rt_changed;
}

bool router::update_dv()
{
	//iterate through rt and make sure dv costs are up to date
	//if everything up to date, return false
	bool prop = false;

	for(std::unordered_map<char, routing_info>::iterator it = rt.begin(); it != rt.end(); it++)
	{
		if(dv[it->first] != (it->second).cost)
		{
			prop=true;
			dv[it->first] = (it->second).cost;
		}
	}
	return prop;
}

void router::handle_dv_update(std::string msg, char sender_name)
{
	updates[sender_name]+=1; //to check for dropped msg
	//std::cout << "Printing update counts" << std::endl;
	//print_dv(&updates);
	std::unordered_map<char, int> rcvd_dv = parse_msg(msg); //get the sent DV
	std::unordered_map<char, routing_info> temp_old_rt(rt); //in case we need to print old rt

	bool rt_changed = update_rt(sender_name, &rcvd_dv);
	bool update_nb = update_dv();
	if(!rt_changed)
		return;

	std::cout << "Recieved updated DV from " << sender_name << std::endl;
	std::cout << "Timestamp: " << time(NULL) << std::endl;
	std::cout << "Routing table before change" << std::endl;
	print_rt(&temp_old_rt);
	
	std::cout << "DV that caused change" << std::endl;
	print_dv(&rcvd_dv);

	std::cout << "New routing table" << std::endl;
	print_rt(&rt);

	//send updated dv to neighbors only if dv changed
	if(!update_nb)
		return;

	std::string update_msg = form_msg('U');
	const char* s_msg = update_msg.c_str();
	struct sockaddr_in nb_info;

	//iterate through neighbors and send msg
	for(int i = 0; i < nb.size(); i++)
	{
		nb_info.sin_family = AF_INET;
		nb_info.sin_port = htons(name_to_port(nb[i])); //nb port
		nb_info.sin_addr.s_addr = htonl(0x7F000001); //localhost
		if(sendto(s,s_msg,strlen(s_msg),0,(struct sockaddr *)&nb_info,sizeof(nb_info)) < 0) 
			std::cout << "send message failed" << std::endl;
		else
			std::cout << "propagated DV to router " << nb[i] << std::endl;
	}
}

void router::handle_forward_msg(std::string msg, char sender_name)
{
	std::cout << "Recieved data packet from " << sender_name << std::endl;
	std::cout << "Timestamp: " << time(NULL) << std::endl;
	std::cout << "Packet arrived from port: " << name_to_port(sender_name)
			  << std::endl; 
	//P-SRCNODE|DESTNODE|DATA
	char destination = msg[4];
	std::cout << "Destination router: " << destination << std::endl;
	if(destination == server_name) //packet meant for us
	{
		std::cout << "Data: " << msg.substr(6) << std::endl;
		std::cout << std::endl;
		return;
	}
	msg[2] = server_name; //update packet to send

	std::unordered_map<char, routing_info>::iterator it = rt.find(destination);
	if (it != rt.end())
	{
		struct sockaddr_in normal_msg;
		normal_msg.sin_family = AF_INET;
		//forward to next_hop
		unsigned short fwd_port = name_to_port((it->second).next_hop);
		std::cout << "Forward port: " << fwd_port << std::endl;

		normal_msg.sin_port = htons(fwd_port);
		normal_msg.sin_addr.s_addr = htonl(0x7F000001); 


		const char* t_msg = msg.c_str();
		if(sendto(s,t_msg,strlen(t_msg),0,(struct sockaddr *)&normal_msg,sizeof(normal_msg)) < 0) 
		{	std::cout << "send message failed" << std::endl;
			exit(1);
		}
	}
	else
		std::cout << "Can not forward packet" << std::endl;
	std::cout << std::endl;
}

void router::handle_death_msg(std::string msg, char sender_name)
{
	std::cout << "Recieved death message from " << sender_name << std::endl;
	std::cout << "Timestamp: " << time(NULL) << std::endl;
	std::cout << "Packet arrived from port: " << name_to_port(sender_name)
			  << std::endl; 

	std::stringstream ss(msg);
	std::string elem;
	getline(ss, elem, '|'); //remove msg header
	bool any_new = false;

	while(getline(ss, elem, '|'))
	{
		//check if already in dead router vector
		if(std::find(dr.begin(),dr.end(),elem[0]) != dr.end())
			continue;
		else
		{
			dr.push_back(elem[0]);
			any_new = true;
		}
	}

	if(any_new) //if any new dead vectors, restart
	{
		initialize();
		broadcast('D');
	}
}

void router::handle_msg(char* t_msg, int msg_len)
{
	std::string msg = t_msg;
	if(msg[0] == 'D') //a DV message
		handle_dv_update(msg, msg[2]);
	else if(msg[0] == 'P')//a packet to forward
	{
		//create function to forward packet
		//packet structure:
		//P-SRCNODE|DESTNODE|DATA
		handle_forward_msg(msg, msg[2]);
	}
	else if (msg[0] == 'K') //a death msg
	{
		handle_death_msg(msg, msg[2]);
	}
	else
	{
		std::cout << "Invalid packet recieved" << std::endl;
		std::cout << std::endl;
	}
}

//return 1 on error, 0 on success
//<source router, destination router, source UDP port, link cost>
void router::initialize()
{
	const char* tp_file = file.c_str();

	dv.clear();
	rt.clear();
	nb.clear();
	updates.clear();
	link_costs.clear();

	std::ifstream tp_info(tp_file);
	if(!tp_info.is_open())
	{
		std::cout << "error opening topology file" << std::endl;
		exit(1);
	}

	std::string line, item;
	routing_info* temp;
	std::vector<std::string> fields(4,"");
	int c=0;
	while(getline(tp_info, line))
	{
		//look for neighbors only
		if(line[0] != server_name)
			continue;

		//skip dead routers
		std::vector<char>::iterator it = std::find(dr.begin(),dr.end(),line[2]);
		if(it != dr.end())
			continue;

		//split by comma
		c=0;
		std::stringstream ss(line);
		while(getline(ss,item,',') && c<4) //add fields to vector
			fields.at(c++) = item;

		//fields[0] is our server name
		//fields[1] is dest server name
		//fields[2] is dest port
		//fields[3] is dest cost

		nb.push_back(fields.at(1)[0]); //add neighbor name

		dv[fields.at(1)[0]] = stoi(fields.at(3)); //store cost
		link_costs[fields.at(1)[0]] = dv[fields.at(1)[0]];

		//update routing table with neighbors
		temp = &rt[fields.at(1)[0]];
		(*temp).cost = dv[fields.at(1)[0]];
		(*temp).out_port = server_port;
		(*temp).dest_port = stoi(fields.at(2));
		(*temp).next_hop = fields.at(1)[0];
	}
}

void router::check_dead()
{
	int avg_updates=0;
	int prev_dr_size = dr.size();

	for(int i=0; i<nb.size(); i++) //get update counts of neighbors
		avg_updates += updates[nb[i]];
	avg_updates /= nb.size();

	for(int i=0; i<nb.size(); i++)
	{
		if(updates[nb[i]] < (avg_updates-3)) //may need to change delta
		{
			dr.push_back(nb[i]);
			std::cout << "Router " << nb[i] << " died" << std::endl;
		}
	}

	if(dr.size() > prev_dr_size)
	{
		initialize();
		//propagate death msg
		broadcast('D');
	}
}

void router::run_router()
{
	std::cout <<"\n\n\nstart-router\n"<<std::endl;
	int count = 0;

	struct sockaddr_in myaddr;
	struct sockaddr_in claddr; //client's info
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		std::cout << "cannot create socket" << std::endl;
		exit(1);
	} 

	//bind socket to specified port
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(0x7F000001); //IP addr is localhost
	myaddr.sin_port = htons(server_port);
	if(bind(s, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
	{
		std::cout << "bind failed" << std::endl;
		exit(1);
	}

	//recieve client data
	socklen_t claddr_len = sizeof(claddr);
	char buf[BUFSIZE];
	int rec_len; //# bytes recieved


	//set timer
	struct  timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	//fd_set for select
	fd_set fds;
	FD_ZERO(&fds);




	while(1) 
	{
		FD_SET(s,&fds);

		int send = select(s+1, &fds,NULL, NULL, &timeout);

		if (send < 0)
		{
			std::cout<<"select failed"<<std::endl;
			exit(1);
		}

		else if (send == 0)
		{
			//send the dvs;

			std::cout<< "\nselect broadcast "<<count<<std::endl;
			check_dead();
			broadcast('U');
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
			count++;
		}
		else{
			if (FD_ISSET(s,&fds))
			{
				std::cout<<"select recieving"<<std::endl;

				std::cout << "Waiting on port: " << server_port << std::endl;
				rec_len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&claddr, &claddr_len);
				std::cout << "Recieved " << rec_len << " bytes" << std::endl;
				if(rec_len > 0) {
					buf[rec_len] = 0;
					std::cout << std::endl;
					//need to handle msg: decide if DV update or packet
					handle_msg(buf,rec_len);
					//test();
		}			
	}
			
	}
	}
}

void router::broadcast(char type)
{
	std::cout<<"broadcast"<<std::endl;
	std::string s_msg = form_msg(type);
	const char* msg = s_msg.c_str();


	int socketn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketn < 0)
	{
		std::cout<<"fail to open the socket"<<std::endl;
		exit(1);
	}

	int option = 1;
	if (setsockopt(socketn, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int))<0) {
		std::cout<<"fail to set the socket option" << std::endl;
    	exit(1);
    }

	for(std::vector<char>::iterator it = nb.begin(); it != nb.end(); it++)
	{
			struct sockaddr_in myaddr;
			memset(&myaddr, 0, sizeof(myaddr));

			myaddr.sin_family = AF_INET;
			myaddr.sin_addr.s_addr = htonl(0); //equivalent to INADDR_ANY
			myaddr.sin_port = htons(name_to_port(*it));
			if (sendto(socketn, msg, strlen(msg), 0, (struct sockaddr *)&myaddr, (socklen_t)sizeof(myaddr)) < 0)	
			{
				std::cout<<server_name<<" fail to send msg to "<<*it<<std::endl;
				exit(1);
			}

			if(type == 'U')
				std::cout<<server_name<<" send dv to "<<*it<<std::endl;
			else
				std::cout<<server_name<<" send death msg to "<<*it<<std::endl;
	}
	std::cout << std::endl;

}

router::router(char srv_name, char* tp_file)
{
	server_name = srv_name;
	server_port = name_to_port(server_name);
	file = tp_file;

	std::string log_name = "routing-output";
	log_name+=server_name;
	log_name+=".txt";

	std::freopen(log_name.c_str(),"w", stdout); //log info

	initialize();
}

router::~router()
{
	std::cout << "router stopping" << std::endl;
}
