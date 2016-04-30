#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "cubelib.h"
#define CLI_NUM 4
#define BUF_SIZE 1024
#define USERNAME_LENGTH 20+1
char buf[BUF_SIZE];
char receive_buf[BUF_SIZE];
int listen_fd;
int pipe_fd[CLI_NUM * 2];
char* online_list[CLI_NUM];
pthread_mutex_t mutex;

void signal_handler(int sig)
{
    int temp_i, temp_i1 = -1;
    switch( sig )
    {
    case SIGINT:
        for( temp_i = 0 ; temp_i < CLI_NUM * 2; temp_i += 2 )
        {
            write_all( pipe_fd[temp_i + 1], &temp_i1, 4 );
            close( pipe_fd[temp_i] );
            close( pipe_fd[temp_i + 1] );
        }
        close( listen_fd );
        exit( 0 );
    }
}

void *transmission( void *ptr )
{
    int fd = *(int*)ptr;
    printf("\ntransmissition : fd_num = %d\n", fd );
    int msg_len;
    int temp_i, temp_i1;
    int fd_list[CLI_NUM];
    int cli_count = 0;
    char trans_buf[BUF_SIZE];
    while( read( fd, &msg_len, 4 ) )
    {
        memset( trans_buf, 0, BUF_SIZE );
        while( msg_len > 0 && ( temp_i = read( pipe_fd[fd], trans_buf, msg_len ) ) )
        {
            printf("\n Master Process Transmission : receive message from Middle_UP\n -- %s\n", trans_buf );
            msg_len -= temp_i;
            if( trans_buf[0] != '"' )
            {
                pthread_mutex_lock( &mutex );
                for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
                {
                    if( strcmp( trans_buf, online_list[temp_i] ) == 0 )
                    {
                        fd_list[cli_count++] = pipe_fd[2 * temp_i + 1];
                    }

                }
                pthread_mutex_unlock( &mutex );
            }
            else
            {
                for( temp_i1 = 0 ; temp_i1 < cli_count ; temp_i1++ )
                {
                    write_all( fd_list[temp_i1], buf, temp_i );
                }
            }
        }
    }
}




int run_srv( char* srv_port )
{
    pthread_mutex_init( &mutex, PTHREAD_MUTEX_TIMED_NP );
    int msg_len = 0;
    int temp_i, temp_i1;
    for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
        online_list[temp_i] = NULL;
    signal( SIGINT, signal_handler );
    char integer[4];
    int r, w;
    pthread_t trans[CLI_NUM];
//--------------------------------------------------------------------------------------------------------------------
//creating listenning socket
    puts("creating listenning socket");
    struct sockaddr_in srv, cli;
    int cli_len = sizeof( cli );
    if( ( listen_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == -1 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = htonl( INADDR_ANY );
    srv.sin_port = htons( atoi( srv_port ) );
    puts("bind");
    if( bind( listen_fd, (struct sockaddr*) &srv, sizeof( srv ) ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    puts("listen");
    if( listen( listen_fd, 5 ) == -1 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }

    int process_id[CLI_NUM], pid;
    int connect_fd;
    int cli_counter = 0;
    char sender[USERNAME_LENGTH];
    char temp_c;
    while( 1 )
    {
//--------------------------------------------------------------------------------------------------------------------
//Main process waits for new connection
        if( ( connect_fd = accept( listen_fd, (struct sockaddr*) &cli, &cli_len ) ) == -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
//--------------------------------------------------------------------------------------------------------------------
//confirm that the connection is success
        printf("\nMaster Process : confirm that the connection is success\n");
        memset( sender, 0, USERNAME_LENGTH );
        if( recv( connect_fd, sender, USERNAME_LENGTH, 0 ) == -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
        printf("\nMaster Process : connect successfully!!\nUser == %s == logs in!!\n", sender );
//--------------------------------------------------------------------------------------------------------------------
//adding this client to online list
        pthread_mutex_lock( &mutex );
        for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
        {
            if( online_list[temp_i] == NULL )
            {
                temp_i1 = strlen( sender ) + 1;
                online_list[temp_i] = (char*)malloc( temp_i1 );
                memset( online_list[temp_i], 0, temp_i1 );
                strcpy( online_list[temp_i], sender );
                pipe( pipe_fd + ( temp_i * 2 ) ) ;
                r = temp_i * 2;
                w = r + 1;
                break;
            }
        }
        pthread_mutex_unlock( &mutex );
        printf("\nfd_num = %d\n", r );
        pthread_create( &trans[cli_counter], NULL, (void*)transmission, (void*)&r );
//--------------------------------------------------------------------------------------------------------------------
//fork a process taking responsibility for connect with client
        if( ( pid = fork() ) == 0 )
        {
            close( listen_fd );
//--------------------------------------------------------------------------------------------------------------------
//fork another process taking responsibility for receving message from main process and sending these message to client
//            printf("\n %s : Middle_Down process : fork another process taking responsibility for receiving message from main process and sending these message to client\n", sender );

            if( fork() == 0 )
            {
                while( 1 )
                {
                    read( pipe_fd[r], &msg_len, 4 );
                    printf("\n %s : Middle_Down process : msg_len : %d", sender, msg_len );
                    if( msg_len == -1 )
                    {
                        close( pipe_fd[r] );
                        close( pipe_fd[w] );
                        close( connect_fd );
                        printf("\n %s : Middle_Down process : process end\n", sender );
                        exit( 0 );
                    }
                    memset( buf, 0, BUF_SIZE );
                    while( msg_len > 0 && ( temp_i = read( pipe_fd[r], buf, msg_len ) ) )
                    {
                        if( temp_i < 0 )
                        {
                            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                            perror("");
                            exit( 1 );
                        }
                        printf("\n %s : Middle_Down process : receive --%s-- from server, strlen = %d \n", sender, buf, temp_i );
                        sendall( connect_fd, buf, &temp_i );
                        printf("\n %s : Middle_Down process : \nsend message : --%s-- to client\n", sender, buf );
                        memset( buf, 0, BUF_SIZE );
                        msg_len -= temp_i;
                    }
                }
            }
//--------------------------------------------------------------------------------------------------------------------
//Middle_UP process : check offline message
            printf("\n%s : Middle_Up process : check offline message\n", sender );
            char username_temp[USERNAME_LENGTH];

            int msg_len;
            FILE* offline_msg;
            if( ( offline_msg = fopen( "offline_message", "r" ) ) != 0 )
            {
                while( !feof( offline_msg ) )
                {
                    fscanf( offline_msg, "%s", username_temp );
                    if( strcmp( sender, username_temp ) == 0 )
                    {
                        fscanf( offline_msg, "%d", &msg_len );

                        while(  msg_len > 0 && ( temp_i = fscanf( offline_msg, "%s", buf ) ) )
                        {
                            msg_len -= temp_i;
                            send( connect_fd, buf, temp_i, 0 );
                        }
                    }
                }
            }
//--------------------------------------------------------------------------------------------------------------------
//Middle_UP process : receving message from client, and sending to Master Process
            while( recv( connect_fd, &msg_len, 4, 0 ) )
            {
                write( pipe_fd[w], &msg_len, 4 );
                memset( buf, 0, BUF_SIZE );
                while( msg_len > 0 && ( temp_i = recv( connect_fd, buf, msg_len, 0 ) ) )
                {
                    printf("\n Middle_UP process : receive message from ~%s~\n--%s--\n ", sender, buf );
                    write( pipe_fd[w], buf, temp_i );
                    msg_len -= temp_i;
                }

            }
//--------------------------------------------------------------------------------------------------------------------
//Middle_Up process : process end
            printf("\n%s : Middle_Up process : User : %s's process end\n", sender, sender );
            for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
            {
                if( online_list[temp_i] == NULL )
                    free( online_list[temp_i] );
            }
            close( pipe_fd[r] );
            close( pipe_fd[w] );
            close( connect_fd );
            exit( 0 );
        }
        else if( pid < 0 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
//            scanf(" %c", &temp_c );
        }
        else
        {
//--------------------------------------------------------------------------------------------------------------------
//Master Process : broadcast that this client is online
            puts("Master Process : broadcast that this client is online");

            memset( buf, 0, BUF_SIZE );
            strcat( buf, sender );
            strcat( buf, " is online!!");
            msg_len = strlen( buf );
            buf[msg_len] = '\0';
            printf("\nMaster Process : broadcast --%s-- to all client, strlen = %d !!\n", buf, msg_len );
            for( temp_i = 0 ; temp_i < cli_counter ; temp_i++ )
            {
                write_all( pipe_fd[2 * temp_i + 1], &msg_len, 4 );
                write_all( pipe_fd[2 * temp_i + 1], buf, msg_len );
            }
        }
        process_id[cli_counter++] = pid;
        close( connect_fd );
    }

    close( listen_fd );
    return 0;
}


int main( int argc, char* argv[])
{
    if( argc == 2 )
        run_srv( argv[1] );

    return 0;
}
