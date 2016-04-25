#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int run_cli( char* srv_ip, char* connect_port, char* user_name )
{
    int fd;
    if( ( fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }

    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons( atoi( connect_port ) );
    srv.sin_addr.s_addr = inet_addr( srv_ip );
    if( connect( fd, (struct sockaddr*) &srv, sizeof( srv ) ) == -1 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    send( fd, user_name, strlen( user_name ), 0 );

    puts("\n----------log in successfully!!----------\n");





    return 0;
}

int main( int argc, char* argv[] )
{
    if( argc == 4 )
        run_cli( argv[1], argv[2], argv[3] );
    return 0;
}
