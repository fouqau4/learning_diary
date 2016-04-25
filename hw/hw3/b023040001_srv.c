#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define cli_num 3
#define BUFF_SIZE 1024
#define USERNAME_LENGTH 20
char buf[BUFF_SIZE];
char sender[USERNAME_LENGTH], tempA_c_USERNAME_LENGTH[USERNAME_LENGTH];

int run_srv( char* srv_port, char *broadcast_port )
{
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

    int process_id[cli_num], pid;
    int connect_fd;
    int cli_counter = 0;
    int b_port = atoi( broadcast_port );
    char temp_c;
    while( 1 )
    {
        if( ( connect_fd = accept( listen_fd, (struct sockaddr*) &cli, &cli_len ) ) == -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }

        if( ( pid = fork() ) == 0 )
        {
            memset( sender, 0, USERNAME_LENGTH );
            memset( tempA_c_USERNAME_LENGTH, 0, USERNAME_LENGTH );
            if( recv( connect_fd, sender, USERNAME_LENGTH, 0 ) == -1 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            puts("connect successfully!!");
            printf("User == %s == logs in!!\n", sender );
            FILE* online_list;
            if( ( online_list = fopen( "online_list", "r" ) ) == 0 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            //--------------------------------------------------------------------------------------------------------------------
            //broadcast that this client is online
            int broadcast_fd;
            if( ( broadcast_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 )
            {
                fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
                perror("");
                exit( 1 );
            }
            struct sockaddr_in broadcast_srv;
            broadcast_srv.sin_port = htons( b_port );
            broadcast_srv.sin_family = AF_INET;
            broadcast_srv.sin_addr.s_addr = htonl( INADDR_ANY );

            while( !feof( online_list ) )
            {
                fscanf( online_list, "%s", tempA_c_USERNAME_LENGTH );
                if( strcmp( sender, tempA_c_USERNAME_LENGTH ) == 0 )
                {
                    fscanf( online_list, "%s %s", tempA_c_USERNAME_LENGTH, buf );

                }

            }

//            FILE* offline_msg;
//            if( ( offline_msg = fopen( "offline_message", "r" ) ) != 0 )
//            {
//                while( !feof( offline_msg ) )
//                {
//                    fgets( buf, BUFF_SIZE, offline_msg );
//                    sscanf( buf, "%s %s %s", sender, tempA_c_USERNAME_LENGTH);
//                }
//            }


            close( connect_fd );
        }
        else if( pid != -1 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
//            scanf(" %c", &temp_c );
        }
        process_id[cli_counter++] = pid;
        b_port++;
        close( connect_fd );
    }

    close( listen_fd );
    return 0;
}


int main( int argc, char* argv[])
{
    if( argc == 3 )
        run_srv( argv[1], argv[2] );

    return 0;
}
