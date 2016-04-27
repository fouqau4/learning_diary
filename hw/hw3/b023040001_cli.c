#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define BUF_SIZE 1024

void receive_from_srv( void* fd )
{
    char receive_buf[BUF_SIZE];
    int srv_fd = *(int*)fd;
    memset( receive_buf, 0, BUF_SIZE );
    while( recv( srv_fd, receive_buf, BUF_SIZE, 0 ) )
    {
        puts("---------------------------------------------------------------------------");
        printf("%s\n", receive_buf );

    }
}

int run_cli( char* srv_ip, char* connect_port,  char* user_name )
{
    //--------------------------------------------------------------------------------------------------------------------
    //create socket
    puts("create sockte");
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
    //--------------------------------------------------------------------------------------------------------------------
    //create a thread taking responsibility for receiving message from server process
    puts("create a thread taking responsibility for receiving message from server process");
    pthread_t cli_receiver;
    pthread_create( &cli_receiver, NULL, (void*)receive_from_srv, (void*) &fd );
    //--------------------------------------------------------------------------------------------------------------------
    send( fd, user_name, strlen( user_name ), 0 );
    puts("\n----------log in successfully!!----------\n");

    char operation;

    while( scanf(" %c", &operation ) )
    {
        if( operation == 'q' )
            break;
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
