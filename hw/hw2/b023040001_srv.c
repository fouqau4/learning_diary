#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "myTcp.h"
//IPPROTO_TCP is defined in <netinet/in.h>
int run_srv(){
    //socket()
	int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	//bind()
	struct sockaddr_in srv;
	srv.sin_family = AF_INET;
	srv.sin_port = htons(80);
	srv.sin_addr.s_addr = htonl( INADDR_ANY );

    if(bind( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("bind error!!\n"), exit(1);
    //listen()
    if(listen( fd, 5) < 0)
        perror("listen error!!\n"), exit(1);
    //accept()
    struct sockaddr_in cli;
    int newfd;
    int cli_len = (int)sizeof(cli);
    if((newfd = accept( fd, (struct sockaddr*) &cli, &cli_len))< 0)
        perror("accept error!!\n"), exit(1);
    //read()

}

