#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

#define PORT 10000
#define BUFSIZE 1024

typedef struct
{
	int cost;
	unsigned short out_port;
	unsigned short dest_port;
} routing_info;

typedef unordered_map<char, int> mapc_int;
typedef unordered_map<char, routing_info> mapc_rt;
typedef vector<string> vstring;
typedef vector<char> vchar;

void test(mapc_int* dv, mapc_rt* rt, vchar* nb)
{
	for(mapc_int::iterator it = (*dv).begin(); it != (*dv).end(); it++)
		cout << "Router " << it->first << ", Cost " << it->second << endl;

	for(mapc_rt::iterator it = (*rt).begin(); it != (*rt).end(); it++)
	{
		cout << "Router" << it->first << endl;
		cout << "\tCost: " << (it->second).cost << endl;
		cout << "\tOutgoing port: " << (it->second).out_port << endl;
		cout << "\tDestination port: " << (it->second).dest_port << endl;
	}

	cout << "Neighbors:" << endl;
	for(int i=0; i<(*nb).size(); i++)
		cout << "\t" << (*nb)[i] << endl;
}

//TODO: more error checking
//return 1 on error, 0 on success
//<source router, destination router, source UDP port, link cost>
int initialize(vchar* nb, mapc_rt* rt, mapc_int* dv, char* tp_file, char srv_name)
{
	ifstream tp_info(tp_file);
	if(!tp_info.is_open())
	{
		cerr << "error opening topology file" << endl;
		return 1;
	}

	string line, item;
	routing_info* temp;
	vstring fields(4,"");
	int c=0;
	while(getline(tp_info, line))
	{
		//look for neighbors only
		if(line[0] != srv_name)
			continue;

		//split by comma
		c=0;
		stringstream ss(line);
		while(getline(ss,item,',') && c<4) //add fields to vector
			fields.at(c++) = item;

		//fields[0] is our server name
		//fields[1] is dest server name
		//fields[2] is dest port
		//fields[3] is dest cost

		nb->push_back(fields.at(1)[0]); //add neighbor name

		(*dv)[fields.at(1)[0]] = stoi(fields.at(3)); //store cost

		//update routing table with neighbors
		temp = &(*rt)[fields.at(1)[0]];
		(*temp).cost = (*dv)[fields.at(1)[0]];
		(*temp).out_port = PORT;
		(*temp).dest_port = stoi(fields.at(2));
	}
	return 0;
}

void create_server()
{
	int s;
	struct sockaddr_in myaddr;
	struct sockaddr_in claddr; //client's info
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		cerr << "cannot create socket" << endl;
		exit(1);
	} 

	//bind socket to specified port
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(0x7F000001); //IP addr is localhost
	myaddr.sin_port = htons(PORT);
	if(bind(s,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
	{
		cerr << "bind failed" << endl;
		exit(1);
	}

	//recieve client data
	socklen_t claddr_len = sizeof(claddr);
	unsigned char buf[BUFSIZE];
	int rec_len; //# bytes recieved

	while(1) 
	{
		cout << "Waiting on port: " << PORT << endl;
		rec_len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&claddr,&claddr_len);
		cout << "Recieved " << rec_len << " bytes" << endl;
		if(rec_len > 0) {
			buf[rec_len] = 0;
			cout << "Message recieved: " << buf << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	//argv[1] is server name (one character only)
	//argv[2] is the topology file
	if(argc != 3) 
	{
		cerr << "missing router name or topology file" << endl;
		return 1;
	}

	//initialize server info and data
	char server_name = argv[1][0];
	mapc_int dv; //map dest name to cost
	mapc_rt rt; //routing table
	vchar nb; //neighbors

	//iterate through topology file and initialize dv
	if(initialize(&nb, &rt, &dv, argv[2],server_name)) //returns 1 on failure
		return 1;

	//REMOVE: testing if dv & rt & nb set up properly
	test(&dv,&rt,&nb);

	//start up the server
	create_server();
}