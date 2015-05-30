#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

#define PORT 10000
#define BUFSIZE 1024

int main()
{
	int s;
	struct sockaddr_in myaddr;
	struct sockaddr_in claddr; //client's info

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		cerr << "cannot create socket" << endl;
		return 0;
	} 

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(0x7F000001); //equivalent to INADDR_ANY
	myaddr.sin_port = htons(PORT);
	if(bind(s,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		cerr << "bind failed" << endl;
		return 0;
	}

	socklen_t claddr_len = sizeof(claddr);
	unsigned char buf[BUFSIZE];
	int rec_len; //# bytes recieved

	while(1) {
		cout << "Waiting on port: " << PORT << endl;
		rec_len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&claddr,&claddr_len);
		cout << "Recieved " << rec_len << " bytes" << endl;
		if(rec_len > 0) {
			buf[rec_len] = 0;
			cout << "Message recieved: " << buf << endl;
		}
	}
}