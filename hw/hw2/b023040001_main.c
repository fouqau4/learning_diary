#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b023040001_srv.h"
#include "b023040001_cli.h"

int main(int argc, char *argv[]){


    if(argc == 2 && strcmp( argv[1], "server" ) == 0)
    {
        puts("server");
        run_srv();
    }
    else if(argc == 4 && strcmp( argv[1], "client" ) == 0)
    {
        puts("client");
        run_cli( argv[2], atoi(argv[3]) );
    }
    else
    {
        puts("Fail to execute this program!!");
        puts("Run server : ./b023040001_ftp server");
        puts("Run client : ./b023040001_ftp client <ip(x.x.x.x)> <port>");
        return 1;
    }

    return 0;
}
