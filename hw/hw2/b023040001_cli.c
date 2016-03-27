#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>


int run_cli(char *srvIp){
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //connect()
    struct sockaddr_in srv;
    srv.sin_addr.s_addr = inet_addr(srvIp);

    if(srv.sin_addr.s_addr == (in_addr_t)-1)
        fprintf(stderr, "inet_addr failed\n"), exit(1);
    //write()

}
