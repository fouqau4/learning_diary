#include "b023040001_cli.h"
#define MAX_BUF_SIZE 1024

int run_cli(char *srvIp, int port ){
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //connect()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(srvIp);

    char buf[MAX_BUF_SIZE];

    if(srv.sin_addr.s_addr == (in_addr_t)-1)
        perror("inet_addr failed\n"), exit(1);

    if(connect( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("connect error!!"), exit(1);
    //write()
    else
    {
        puts("connect success!!");
        puts("please enter the string you want to send :");
        scanf("%s",buf);
        write( fd, buf, sizeof(buf) );
        close(fd);
    }

    return 0;
}
