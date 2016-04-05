#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b023040001_srv.h"
#define MAX_BUF_SIZE 1024
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
    {
        perror("bind error!!\n");
        close(fd);
        exit(1);
    }
    //listen()
    if(listen( fd, 5) < 0)
    {
        perror("listen error!!\n");
        close(fd);
        exit(1);
    }
    //select()

    fd_set readfds;
    struct timeval waitCli = {100,0};

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    if(select( fd+1, &readfds, 0, 0, &waitCli ) < 0)
    {
        perror("select error!!\n");
        close(fd);
        exit(1);
    }

    //accept()
    int newfd;
    struct sockaddr_in cli;
    int cli_len = (int)sizeof(cli);
    unsigned char buf[MAX_BUF_SIZE]; memset( buf, 0, sizeof(buf) );

    if(FD_ISSET( fd, &readfds ))
    {
        puts("fd free!!");
        if((newfd = accept( fd, (struct sockaddr*) &cli, &cli_len))< 0)
        {
            perror("accept error!!\n");
            close(fd);
            exit(1);
        }
    //read()
        else
        {
            puts("client sent this :");
            read( newfd, buf, sizeof(buf) );
            puts(buf);
            read( newfd, buf, sizeof(buf) );
            puts(buf);
        }
    }
    else
        puts("server is inpatient :(");

    //close()
    close(newfd);
    close(fd);
    return 0;
}

