#include "client_header.h"


int main()
{
	InitDataBase();

	int sockfd = InitTcp();

	main_handler(sockfd);

    return 0;
}

