#include "my-router.h"
#include <cstdio>
#include <time.h>
#include <sys/time.h>

//TODO: port should be read from topology file
//#define PORT 10000
#define BUFSIZE 1024

void router::test()
{
	for(mapc_int::iterator it = dv.begin(); it != dv.end(); it++)
		std::cout << "Router " << it->first << ", Cost " << it->second << std::endl;

	for(mapc_rt::iterator it = rt.begin(); it != rt.end(); it++)
	{
		std::cout << "Router" << it->first << std::endl;
		std::cout << "\tCost: " << (it->second).cost << std::endl;
		std::cout << "\tOutgoing port: " << (it->second).out_port << std::endl;
		std::cout << "\tDestination port: " << (it->second).dest_port << std::endl;
		std::cout << "\tNext hop: " << (it->second).next_hop << std::endl;
	}

	std::cout << "Neighbors:" << std::endl;
	for(int i=0; i<nb.size(); i++)
		std::cout << "\t" << nb[i] << std::endl;

	std::string msg = form_msg();
	std::cout << "DV msg format: " << msg << std::endl;

	mapc_int tdv = parse_msg(msg);
	std::cout << "Parsed info: \n";
	for(mapc_int::iterator it = tdv.begin(); it != tdv.end(); it++)
		std::cout << "Router " << it->first << ", Cost " << it->second << std::endl;
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

std::string router::form_msg()
{
	std::string msg;
	msg+="D-"; //D at front indicates that this is a DV update
	msg+=server_name; //check where msg is coming from
	for(mapc_int::iterator it = dv.begin(); it != dv.end(); it++)
	{
		msg+='|';
		msg+=it->first;
		msg+='/'+std::to_string(it->second);	
	}
	return msg;
}

//TODO: error checking
mapc_int router::parse_msg(std::string msg)
{
	mapc_int rcvd_dv; //to create dv sent over
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

void router::update_rt(char sender_name, mapc_int* rcvd_dv) 
{
    // Get the node information
    int costFromTable, newCost, currentCost;
    char currentNode;
    bool exists;

    int costToNode = rt[sender_name].cost;

    // Iterate over the distance vector
    mapc_int::iterator it;
    for(it = (*rcvd_dv).begin(); it != (*rcvd_dv).end(); it++) 
    {
        currentNode = it->first;
        currentCost = it->second;

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
                }
                else if (newCost == costFromTable && sender_name < rt[currentNode].next_hop) 
                    rt[currentNode].next_hop = sender_name;
            }
            else {
                // Add to the table
                rt[currentNode].cost = costToNode + currentCost;
                rt[currentNode].next_hop = sender_name;
                rt[currentNode].out_port = server_port;
                rt[currentNode].dest_port = name_to_port(currentNode);
            }
        }
    }
}

bool router::update_dv()
{
	//iterate through rt and make sure dv costs are up to date
	//if everything up to date, return false
	bool prop = false;

	for(mapc_rt::iterator it = rt.begin(); it != rt.end(); it++)
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
	mapc_int rcvd_dv = parse_msg(msg); //get the sent DV
	update_rt(sender_name, &rcvd_dv);
	bool update_nb = update_dv();

	//send updated dv to neighbors only if dv changed
	if(!update_nb)
		return;

	std::string update_msg = form_msg();
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

void router::handle_msg(char* t_msg, int msg_len)
{
	std::string msg = t_msg;
	if(msg[0] == 'D') //a DV message
		handle_dv_update(msg, msg[2]);
	else //a packet to forward
	{
		//create function to forward packet
		//TODO: define packet structure
	}
}

void router::handle_forward_msg(char* t_msg, char destination)
{
	mapc_rt::iterator it = rt.find(destination);
	if (it != rt.end())
	{
		struct sockaddr_in normal_msg;
		normal_msg.sin_family = AF_INET;
		//forward to next_hop
		unsigned short fwd_port = name_to_port((it->second).next_hop);
		normal_msg.sin_port = htons(fwd_port);
		normal_msg.sin_addr.s_addr = htonl(0x7F000001); 

		if(sendto(s,t_msg,strlen(t_msg),0,(struct sockaddr *)&normal_msg,sizeof(normal_msg)) < 0) 
			std::cout << "send message failed" << std::endl;
		
	}

}



//TODO: more error checking
//return 1 on error, 0 on success
//<source router, destination router, source UDP port, link cost>
void router::initialize(char* tp_file)
{
	std::ifstream tp_info(tp_file);
	if(!tp_info.is_open())
	{
		std::cout << "error opening topology file" << std::endl;
		exit(1);
	}

	std::string line, item;
	routing_info* temp;
	vstring fields(4,"");
	int c=0;
	while(getline(tp_info, line))
	{
		//look for neighbors only
		if(line[0] != server_name)
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

		//update routing table with neighbors
		temp = &rt[fields.at(1)[0]];
		(*temp).cost = dv[fields.at(1)[0]];
		(*temp).out_port = server_port;
		(*temp).dest_port = stoi(fields.at(2));
		(*temp).next_hop = fields.at(1)[0];
	}
}


void router::run_router()
{
	std::cout <<"\n\n\nstart-router"<<std::endl;
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

			std::cout<< "\nselct broadcast"<<count<<std::endl;
			broadcast();
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
			count++;
		}
		else{
			if (FD_ISSET(s,&fds))
			{
				std::cout<<"select receiving"<<std::endl;

				std::cout << "Waiting on port: " << server_port << std::endl;
				rec_len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&claddr, &claddr_len);
				std::cout << "Recieved " << rec_len << " bytes" << std::endl;
				if(rec_len > 0) {
					buf[rec_len] = 0;
					std::cout << "Message recieved: " << buf << std::endl;
					//need to handle msg: decide if DV update or packet
					handle_msg(buf,rec_len);
		}			
	}
			
	}
	}
}

void router::broadcast()
{
	std::cout<<"broadcast"<<std::endl;
	std::string s_msg = form_msg();
	const char* msg = s_msg.c_str();


	int socketn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketn < 0)
	{
		std::cout<<"fail to open the socket"<<std::endl;
		exit(1);
	}

	int option = 1;
	if (setsockopt(socketn, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int))<0) {
		std::cout<<"fail to set the socket option";
    	exit(1);
    }

	for(vchar::iterator it = nb.begin(); it != nb.end(); it++)
	{
			struct sockaddr_in myaddr;
			memset(&myaddr, 0, sizeof(myaddr));

			myaddr.sin_family = AF_INET;
			myaddr.sin_addr.s_addr = htonl(0); //equivalent to INADDR_ANY
			myaddr.sin_port = htons(name_to_port(*it));
			if (sendto(socketn, msg, strlen(msg), 0, (struct sockaddr *)&myaddr, (socklen_t)sizeof(myaddr)))	
			{
				std::cout<<server_name<<"fail to send dv to "<<*it<<std::endl;
			}

			std::cout<<server_name<<"send dv to "<<*it<<std::endl;
	}


}

router::router(char srv_name, char* tp_file)
{
	server_name = srv_name;
	server_port = name_to_port(server_name);

	std::string log_name = "routing-output";
	log_name+=server_name;
	log_name+=".txt";

	std::freopen(log_name.c_str(),"w", stdout); //log info

	initialize(tp_file);
}

router::~router()
{
	std::cout << "router stopping" << std::endl;
}
