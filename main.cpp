#include "my-router.h"
#include <iostream>

int main(int argc, char* argv[])
{
	//argv[1][0] is server name (one character only)
	//argv[2] is the topology file
	if(argc != 3) 
	{
		std::cerr << "missing router name or topology file" << std::endl;
		return 1;
	}

	router r1(argv[1][0], argv[2]);
	r1.test();
	r1.run_router();	
}
