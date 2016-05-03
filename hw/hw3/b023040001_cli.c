#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include "cubelib.h"
#define BUF_SIZE 1024
char buf[BUF_SIZE];
int fd;

void receive_from_srv( void *ptr )
{
    sleep( 1 );
    int msg_len;
    int temp_i;
    char *user_name = (char*)ptr;
    char receive_buf[BUF_SIZE];
    memset( receive_buf, 0, BUF_SIZE );
    while( msg_len = recv( fd, receive_buf, BUF_SIZE, 0 ) )
    {
        printf("[%s] << %s\n", user_name, receive_buf );
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
    pthread_create( &cli_receiver, NULL, (void*)receive_from_srv, (void*)user_name );

    //--------------------------------------------------------------------------------------------------------------------
    send( fd, user_name, strlen( user_name ), 0 );
    puts("\n----------log in successfully!!----------\n");

    char operation[10];
    time_t t;
    while( 1 )
    {
        scanf(" %s", operation );
        time( &t );
        temp_i = sizeof( t );
        if( strcmp( operation, "bye" ) == 0 )
        {
            temp_i = 3;
            sendall( fd, operation, &temp_i );
            break;
        }
        else if( strcmp( operation, "chat" ) == 0 )
        {
            temp_i = 4;
            sendall( fd, operation, &temp_i );
            memset( buf, 0, BUF_SIZE );
            gets( buf );

            strcat( buf, ctime( &t ) );
            msg_len = strlen( buf );
            sendall( fd, buf, &msg_len);
        }
        else
        {
            printf("\n<USER HINT>\n");
            printf("\tHi dear %s, you can do two command :\n", user_name );
            printf("\t1. 'bye' ,  to log out\n");
            printf("\t2. 'chat <user1> <user2> ... <userN> \"MESSAGE\"' , to chat with other user\n");
        }
    }
    puts("byebye");
    close( fd );

    return 0;
}

int main( int argc, char* argv[] )
{
    if( argc == 4 )
        run_cli( argv[1], argv[2], argv[3] );
    return 0;
}
