#include "my-router.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <cstring>

#define PORT 12005
#define BUFSIZE 1024

unsigned short get_port(char name)
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

int client(char src, char dest, char inject_to)
{
	int s;
	struct sockaddr_in myaddr;
	struct sockaddr_in servaddr; //client's info

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr << "cannot create socket" << std::endl;
		return 0;
	}

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(0); //equivalent to INADDR_ANY
	myaddr.sin_port = htons(PORT);
	if(bind(s,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		std::cerr << "bind failed" << std::endl;
		return 0;
	}


	//P-SRCNODE|DESTNODE|DATA
	std::string t_msg = "P-E|F|hi there, what's up!!";
	t_msg[2] = src;
	t_msg[4] = dest;

	const char* msg = t_msg.c_str();
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(get_port(inject_to)); //s
	servaddr.sin_addr.s_addr = htonl(0x7F000001);
	if(sendto(s,msg,strlen(msg),0,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
		std::cout << "send message failed" << std::endl;
		return 0;
	}

	return 0;

}

int main(int argc, char* argv[])
{
	//argv[1][0] is server name (one character only)
	//argv[2] is the topology file
	//main SRV_NAME TPLG_FILE
	if(argc < 3)
	{
		std::cerr << "missing router name or topology file" << std::endl;
		return 1;
	}

	//data injector: take SRC DEST as cmd line args
	//main H SRC DEST INJECT_TO
	if(argv[1][0] == 'H') //act as data injector
		return client(argv[2][0], argv[3][0], argv[4][0]);

	router r1(argv[1][0], argv[2]);
	r1.test();
	r1.run_router();
}
