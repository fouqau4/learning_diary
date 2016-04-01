#include "b023040001_srv.h"
//IPPROTO_TCP is defined in <netinet/in.h>
int run_srv()
{
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //bind()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(8787);
    srv.sin_addr.s_addr = htonl( INADDR_ANY );



    if(bind( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("bind error!!\n");//, exit(1);
    //listen()
    if(listen( fd, 5) < 0)
        perror("listen error!!\n"), exit(1);
    //select()

    fd_set readfds;
    struct timeval waitCli = {100,0};

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    if(select( fd+1, &readfds, 0, 0, &waitCli ) < 0)
        perror("select error!!\n"), exit(1);

    //accept()
    int newfd;
    struct sockaddr_in cli;
    int cli_len = (int)sizeof(cli);
    if(FD_ISSET( fd, &readfds ))
    {
        puts("fd free!!");
        if((newfd = accept( fd, (struct sockaddr*) &cli, &cli_len))< 0)
            perror("accept error!!\n"), exit(1);
    }
    else
        puts("server is inpatient :(");

    //read()
    //close()
    close(newfd);
    close(fd);
    return 0;
}

