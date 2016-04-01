#include <stdio.h>
#include <string.h>
#include "b023040001_srv.h"
int main(int argc, char *argv[]){


    if(argc == 2 && strcmp( argv[1], "server" ) == 0)
    {
        puts("server");
        run_srv();
    }
    else if(argc == 4 && strcmp( argv[1], "connect" ) == 0)
        puts("client");
    else
    {
        puts("Fail to execute this program!!");
        puts("Run server : ./b023040001_ftp server");
        puts("Run client : ./b023040001_ftp connect <ip(x.x.x.x)> <port>");
        return 1;
    }

    return 0;
}
