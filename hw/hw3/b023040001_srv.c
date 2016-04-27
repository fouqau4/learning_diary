#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define CLI_NUM 3
#define BUF_SIZE 1024
#define USERNAME_LENGTH 20+1
char buf[BUF_SIZE];
char receive_buf[BUF_SIZE];


int run_srv( char* srv_port )
{
    int pipe_fd[CLI_NUM * 2];
    int msg_len = 0;
    int temp_i;
    //--------------------------------------------------------------------------------------------------------------------
    //creating listenning socket
    puts("creating listenning socket");
    int listen_fd;
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
    if( bind( listen_fd, (struct sockaddr*) &srv, sizeof( srv ) ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }

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
        if( ( connect_fd = accept( listen_fd, (struct sockaddr*) &cli, &cli_len ) ) == -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
        pipe( pipe_fd + ( cli_counter * 2 ) );
        if( ( pid = fork() ) == 0 )
        {
            int r = cli_counter * 2;
            close( listen_fd );
            //--------------------------------------------------------------------------------------------------------------------
            //confirm that the connection is success
            puts("confirm that the connection is success");
            memset( sender, 0, USERNAME_LENGTH );
            if( recv( connect_fd, sender, USERNAME_LENGTH, 0 ) == -1 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            puts("connect successfully!!");
            printf("User == %s == logs in!!\n", sender );
            //--------------------------------------------------------------------------------------------------------------------
            //fork another process taking responsibility for receving message from main process and sending these message to client
            puts("fork another process taking responsibility for receiving message from main process and sending these message to client");
            if( fork() == 0 )
            {
                while( 1 )
                {
                    read( pipe_fd[r], &msg_len, 4 );
                    if( msg_len == -1 )
                    {
                        close( pipe_fd[r] );
                        close( pipe_fd[r + 1] );
                        close( connect_fd );
                        exit( 0 );
                    }
                    while( msg_len > 0 && ( temp_i = read( pipe_fd[r], buf, BUF_SIZE ) ) )
                    {
                        if( temp_i < 0 )
                        {
                            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                            perror("");
                            exit( 1 );
                        }
                        write( connect_fd, buf, temp_i );
                    }
                }
            }
            //--------------------------------------------------------------------------------------------------------------------
            //check offline message
            puts("check offline message");
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
            puts("process exit");
            close( pipe_fd[r] );
            close( pipe_fd[r + 1] );
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

            FILE* online_list;
            if( ( online_list = fopen( "online_list", "r" ) ) == 0 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            memset( buf, 0, BUF_SIZE );
            strcat( buf, sender );
            strcat( buf, " is online!!");
            msg_len = strlen( buf );
            for( temp_i = 0 ; temp_i < cli_counter ; temp_i++ )
            {
                write( pipe_fd[2 * cli_counter - 1], buf, msg_len );
            }


            fclose( online_list );
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
