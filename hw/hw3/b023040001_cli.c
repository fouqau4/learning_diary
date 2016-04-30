#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "cubelib.h"
#define BUF_SIZE 1024
char buf[BUF_SIZE];
int fd;

void receive_from_srv()
{
    char receive_buf[BUF_SIZE];
    memset( receive_buf, 0, BUF_SIZE );
    while( recv( fd, receive_buf, BUF_SIZE, 0 ) )
    {
        puts("--------------------------------------------------------------");
        printf("%s\n", receive_buf );
        memset( receive_buf, 0, BUF_SIZE );
    }
    pthread_exit( NULL );
}

int run_cli( char* srv_ip, char* connect_port,  char* user_name )
{
    int temp_i;
    int msg_len;
    //--------------------------------------------------------------------------------------------------------------------
    //create socket
    puts("create socktet");
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
    if( connect( fd, (struct sockaddr*) &srv, sizeof( srv ) ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    //--------------------------------------------------------------------------------------------------------------------
    //create a thread taking responsibility for receiving message from server process
    puts("create a thread taking responsibility for receiving message from server process");
    pthread_t cli_receiver;
    pthread_create( &cli_receiver, NULL, (void*)receive_from_srv, NULL );
    //--------------------------------------------------------------------------------------------------------------------
    send( fd, user_name, strlen( user_name ), 0 );
    puts("\n----------log in successfully!!----------\n");

    char operation[10];
    temp_i = 4;
    while( scanf(" %s", operation ) )
    {
        if( strcmp( operation, "bye" ) == 0 )
            break;
        if( strcmp( operation, "chat" ) == 0 )
        {
            while( scanf(" %s", buf ) )
            {
                msg_len = strlen( buf );
                sendall( fd, (char*)&msg_len, &temp_i );
                sendall( fd, buf, &msg_len);
                if( buf[0] == '"' )
                    break;
            }
        }
    }

    close( fd );

    return 0;
}

int main( int argc, char* argv[] )
{
    if( argc == 4 )
        run_cli( argv[1], argv[2], argv[3] );
    return 0;
}
