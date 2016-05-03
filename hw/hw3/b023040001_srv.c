#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include "cubelib.h"
#define CLI_NUM 4
#define USER_NUM 20
#define BUF_SIZE 1024
#define USERNAME_LENGTH 20+1

struct user_state
{
    char* user;
    int online;
};

int listen_fd;
int connect_fd[CLI_NUM];
char* online_list[CLI_NUM];
struct user_state user_list[CLI_NUM];

pthread_mutex_t lock_offline_msg;
pthread_mutex_t lock_user_list;

void signal_handler(int sig)
{
    int temp_i, temp_i1 = -1;
    switch( sig )
    {
    case SIGINT:
        close( listen_fd );
        exit( 0 );
    }
}

void *Slave_thread( void *ptr )
{
    char sender[USERNAME_LENGTH];
    strcpy( sender, (char*)ptr );
    char buf[BUF_SIZE], receive_buf[BUF_SIZE];
    int local;
    int msg_len;
    int temp_i, temp_i1;
//--------------------------------------------------------------------------------------------------------------------
//Thread : broadcast that this client is online
    printf("\n[%s] : broadcast that : %s is online!!\n", sender, sender );

    memset( buf, 0, BUF_SIZE );
    strcat( buf, sender );
    strcat( buf, " is online!!");
    msg_len = strlen( buf );
    buf[msg_len] = '\0';
    printf("\n[%s] : broadcast --%s-- to all client, strlen = %d !!\n", sender, buf, msg_len );
    for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
    {
        if( user_list[temp_i].user != NULL && user_list[temp_i].online )
        {
            sendall( connect_fd[temp_i], buf, &msg_len );
            if( strcmp( user_list[temp_i].user, sender ) == 0 )
            {
                local = temp_i;
            }
        }
    }
//--------------------------------------------------------------------------------------------------------------------
//Thread : check offline message
    printf("\n[%s] : check offline message\n", sender );
    memset( buf, 0, BUF_SIZE );
    strcat( buf, sender );
    strcat( buf, ".offline" );
    pthread_mutex_lock( &lock_offline_msg );
    FILE* offline_msg;
    offline_msg = fopen( buf, "r" );
    if( offline_msg < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    if( offline_msg > 0 )
    {
        while( !feof( offline_msg ) )
        {
            memset( buf, 0, BUF_SIZE );
            fgets( buf, BUF_SIZE, offline_msg );
            printf("\n[%s] : get offline msg --%s--\n", sender, buf );
            msg_len = strlen( buf );
            sendall( connect_fd[local], buf, &msg_len );
        }
        fclose( offline_msg );
    }
    pthread_mutex_unlock( &lock_offline_msg );
//--------------------------------------------------------------------------------------------------------------------
//Message transmission
    printf("\n[%s] : [Trans] \n", sender );
    int online_target[CLI_NUM], offline_target[CLI_NUM];
    int online_num, offline_num;
    char *tok, *target_tok, *msg_time;
    char target[USERNAME_LENGTH + 8];
    time_t t;
    while( 1 )
    {
        memset( receive_buf, 0, BUF_SIZE );
        temp_i = recv( connect_fd[local], receive_buf, BUF_SIZE, 0 );
        puts(receive_buf);


        printf("[%s] : [UP] : receving message from client\n--%s--\n ", sender, receive_buf );
        if( strcmp( receive_buf, "bye" ) == 0 )
        {
            break;
        }
        if( strcmp( receive_buf, "chat" ) == 0 )
        {
            puts("chat!!");
            pthread_mutex_lock( &lock_user_list );
            online_num = 0, offline_num = 0;
            puts("wait!!");
            temp_i = recv( connect_fd[local], receive_buf, BUF_SIZE, 0 );
            puts(receive_buf);
            printf("[%s] : receive --%s--\n", sender, receive_buf );
            tok = strtok( receive_buf, "\"" );
            strcpy( buf, tok );
            tok = strtok( NULL, "\"" );
            if( ( msg_time = strtok( NULL, "\"" ) ) == NULL )
            {
                memset( buf, 0, BUF_SIZE );
                strcpy( buf, "unexpected command !! type \"help\" to get hints" );
                temp_i = strlen( buf );
                sendall( connect_fd[local], buf, &temp_i );
                continue;
            }

            puts(tok);
            target_tok = strtok( buf, " " );
            while( target_tok != NULL )
            {
                for( temp_i1 = 0 ; temp_i1 < CLI_NUM ; temp_i1++ )
                {
                    if( user_list[temp_i1].user == NULL )
                    {
                        memset( buf, 0, BUF_SIZE );
                        strcpy( buf, "non-existed user!!" );
                        temp_i = strlen( buf );
                        sendall( connect_fd[local], buf, &temp_i );
                        break;
                    }
                    if( strcmp( user_list[temp_i1].user, target_tok ) == 0 )
                    {
                        if( user_list[temp_i1].online )
                        {
                            printf("[%s] : find online target %s\n", sender, target_tok );
                            online_target[online_num++] = temp_i1;

                        }
                        else
                        {
                            printf("[%s] : find offline target %s\n", sender, target_tok );
                            offline_target[offline_num++] = temp_i1;
                        }
                        break;
                    }
                }
                target_tok = strtok( NULL, " " );
            }
            memset( buf, 0, BUF_SIZE );
            strcat( buf, "[");
            strcat( buf, sender );
            strcat( buf, "] : <" );
            strcat( buf, tok );
            strcat( buf, "> " );
            msg_len = strlen( buf );
            for( temp_i1 = 0 ; temp_i1 < online_num ; temp_i1++ )
            {
                sendall( connect_fd[online_target[temp_i1]], buf, &msg_len );
            }
            pthread_mutex_lock( &lock_offline_msg );
            for( temp_i1 = 0 ; temp_i1 < offline_num ; temp_i1++ )
            {
                memset( target, 0, sizeof( target ) );
                strcat( target, user_list[offline_target[temp_i1]].user );
                strcat( target, ".offline" );
                if( ( offline_msg = fopen( target, "a+" ) ) == 0 )
                {
                    fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                    perror("");
                    exit( 1 );
                }
                strcat( buf, ", " );
                strcat( buf, msg_time );
                puts(buf);
                fprintf( offline_msg, "%s\n", buf );
                fclose( offline_msg );
            }
            pthread_mutex_unlock( &lock_offline_msg );
            pthread_mutex_unlock( &lock_user_list );
        }
        printf("[%s] : [Down] : sending message to Target client\n--%s--\n ", sender, buf );
    }
//--------------------------------------------------------------------------------------------------------------------
//Thread  end
    printf("\n[%s] : User : %s's thread end\n", sender, sender );
    pthread_mutex_lock( &lock_user_list );
    user_list[local].online = 0;
    pthread_mutex_unlock( &lock_user_list );
    close( connect_fd[local] );
//--------------------------------------------------------------------------------------------------------------------
//Thread : broadcast that this client is going to be offline
    printf("\n[%s] : broadcast that : %s is going to be offline!!\n", sender, sender );

    memset( buf, 0, BUF_SIZE );
    strcat( buf, sender );
    strcat( buf, " is offline!!");
    msg_len = strlen( buf );
    buf[msg_len] = '\0';
    printf("\n[%s] : broadcast --%s-- to all client, strlen = %d !!\n", sender, buf, msg_len );
    for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
    {
        if( user_list[temp_i].user != NULL && user_list[temp_i].online )
        {
            sendall( connect_fd[temp_i], buf, &msg_len );
        }
    }
    pthread_exit( NULL );
}


int run_srv( char* srv_port )
{
    pthread_mutex_init( &lock_offline_msg, PTHREAD_MUTEX_TIMED_NP );
    pthread_mutex_init( &lock_user_list, PTHREAD_MUTEX_TIMED_NP );
    int temp_i, temp_i1, temp_i2, temp_i3;
    int user_counter = 0;
    char buf[BUF_SIZE];
    for( temp_i = 0 ; temp_i < CLI_NUM ; temp_i++ )
    {
        user_list[temp_i].user = NULL;
        user_list[temp_i].online = 0;
    }
    signal( SIGINT, signal_handler );
    int fd_list[CLI_NUM];
    pthread_t slave[CLI_NUM];
    //--------------------------------------------------------------------------------------------------------------------
    FILE* u_list;
    u_list = fopen( "user_list", "r" );
    if( u_list < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    if( u_list > 0 )
    {
        memset( buf, 0, BUF_SIZE );
        for( temp_i = 0 ; !feof( u_list ) ; temp_i++ )
        {
            fscanf( u_list, "%s\n", buf );
            user_list[temp_i].user = (char*)malloc( strlen( buf ) + 1 );
            memset( user_list[temp_i].user, 0, strlen( buf ) + 1 );
            strcpy( user_list[temp_i].user, buf );
            user_list[temp_i].online = 0;
            user_counter++;
            printf("old user : %s\n", buf );
        }
        fclose( u_list );
    }
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
//--------------------------------------------------------------------------------------------------------------------
//waiting for connection
    puts("listen");
    if( listen( listen_fd, 5 ) == -1 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//--------------------------------------------------------------------------------------------------------------------

    int position;
    char sender[USERNAME_LENGTH];
    char temp_c;
    int temp_fd;

    while( 1 )
    {
//--------------------------------------------------------------------------------------------------------------------
//confirm that the connection is successful
        if( ( temp_fd = accept( listen_fd, (struct sockaddr*) &cli, &cli_len ) ) == -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
        memset( sender, 0, USERNAME_LENGTH );
        if( recv( temp_fd, sender, USERNAME_LENGTH, 0 ) < 0 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
        printf("\nMain thread : User == %s == logs in!!\n", sender );
//--------------------------------------------------------------------------------------------------------------------
//find position
        for( temp_i = 0, position = -1 ; temp_i < user_counter ; temp_i++ )
        {
            if( strcmp( user_list[temp_i].user, sender ) == 0 )
            {
                position = temp_i;
                user_list[temp_i].online = 1;
                printf("old user : %s log in\n", user_list[temp_i].user );
                break;
            }
        }

//--------------------------------------------------------------------------------------------------------------------
//adding this client to user list
        if( position == -1 && user_counter < CLI_NUM )
        {
            pthread_mutex_lock( &lock_user_list );
            position = temp_i;
            if( ( u_list = fopen( "user_list", "a+" ) ) == 0 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            fprintf( u_list, "%s\n", sender );
            fclose( u_list );

            temp_i = strlen( sender ) + 1;
            user_list[position].user = (char*)malloc( temp_i );
            memset( user_list[position].user, 0, temp_i );
            strcpy( user_list[position].user, sender );
            user_list[position].online = 1;
            user_counter++;
            printf("\nMain Thread : %s is added in online list!!\n", sender );
            pthread_mutex_unlock( &lock_user_list );
        }
        puts("create thread!!");
        connect_fd[position] = temp_fd;
        pthread_create( &slave[position], NULL, (void*)Slave_thread, (void*)sender);

        puts("NEXT");
    }
//--------------------------------------------------------------------------------------------------------------------
    close( listen_fd );
    return 0;
}


int main( int argc, char* argv[])
{
    if( argc == 2 )
        run_srv( argv[1] );
    puts("WTF");
    return 0;
}
