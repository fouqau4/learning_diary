#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b023040001_srv.h"
#include "b023040001_cli.h"

int main(int argc, char *argv[])
{

    switch( argc )
    {
    case 3:
        if( strcmp( argv[1], "server" ) == 0 )
        {
            if( atoi( argv[2] ) >= 1024 )
            {
                run_srv( argv[2] );
                return 0;
            }
            break;
        }
    case 4:
        if( strcmp( argv[1], "client" ) == 0 )
        {
            run_cli( argv[2], argv[3] );
            return 0;
        }
    }

    puts("Fail to execute this program!!");
    puts("Run server : ./b023040001_ftp server <port>");
    puts("Run client : ./b023040001_ftp client <ip(x.x.x.x)> <port>");
    return 1;
}
