#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
using namespace std;

#define PORT 10001
#define BUFSIZE 1024

int main()
{
	int s;
	struct sockaddr_in myaddr;
	struct sockaddr_in servaddr; //client's info

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		cerr << "cannot create socket" << endl;
		return 0;
	} 

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(0); //equivalent to INADDR_ANY
	myaddr.sin_port = htons(PORT);
	if(bind(s,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		cerr << "bind failed" << endl;
		return 0;
	}

	//server info
	socklen_t servaddr_len = sizeof(servaddr);
	char* msg = "Hello this is a test";
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(10000);
	servaddr.sin_addr.s_addr = htonl(0x7F000001);
	if(sendto(s,msg,strlen(msg),0,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
		cout << "send message failed" << endl;
		return 0;
	}
	
}