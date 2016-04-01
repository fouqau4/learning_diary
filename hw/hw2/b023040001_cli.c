#include <sys/socket.h>
#include <netinet/in.h>
#include "b023040001_cli.h"

int run_cli(char *srvIp, int port ){
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //connect()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(srvIp);
    if(srv.sin_addr.s_addr == (in_addr_t)-1)
        perror("inet_addr failed\n"), exit(1);

    if(connect( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("connect error!!"), exit(1);
    else
    {
        puts("connect success!!");
        close(fd);
    }

    //write()
    return 0;
}
