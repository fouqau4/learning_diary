#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b023040001_srv.h"
#define MAX_FILENAME_SIZE 1024
#define MAX_BUF_SIZE 1024
//#define DEBUG

//IPPROTO_TCP is defined in <netinet/in.h>

int run_srv( char* port )
{
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //bind()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons( (u_short)atoi( port ) );
    srv.sin_addr.s_addr = htonl( INADDR_ANY );



    if(bind( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
    {
        perror("bind error!!\n");
        close(fd);
        exit(1);
    }
    puts("\n∥=========================================∥");
    puts("∥       succeed in building socket !!!    ∥");
    puts("∥=========================================∥\n");

    int newfd;
    struct sockaddr_in cli;
    int cli_len = (int)sizeof(cli);
    char ctrl = 'n';
    while( ctrl == 'n' )
    {
        puts("\nooooooooooooooooooooooooooooooooooooooooooo");
        puts("Oo        waiting for connection         oO");
        puts("ooooooooooooooooooooooooooooooooooooooooooo\n");

        //listen()
        if(listen( fd, 5) < 0)
        {
            perror("listen error!!\n");
            close(fd);
            exit(1);
        }
        //select()
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval waitCli = {100,0};
        if(select( fd+1, &readfds, 0, 0, &waitCli ) < 0)
        {
            perror("select error!!\n");
            shutdown( fd, 2 );
            close(fd);
            exit(1);
        }

        if(FD_ISSET( fd, &readfds ))
        {
            //accept()
            if((newfd = accept( fd, (struct sockaddr*) &cli, &cli_len))< 0)
            {
                perror("accept error!!\n");
                shutdown( fd, 2 );
                close(fd);
                exit(1);
            }
            //read()
            else
            {
                int temp_i;
                char cliName[16]; char cliPort[7];
                getnameinfo( (struct sockaddr*) &cli, cli_len, cliName, sizeof(cliName), cliPort, sizeof(cliPort), NI_NUMERICHOST | NI_NUMERICSERV );
                puts("\n∥=========================================∥");
                puts("∥   succeed in connecting with client!!!  ∥");
                printf("∥\t    client information :    \t  ∥\n");
                printf("∥   ip = %s , port = ", cliName );
                for( temp_i = 0 ; temp_i < 7 ; temp_i++ )
                    printf("%c", cliPort[temp_i] );
                puts("   ∥");
                puts("∥=========================================∥\n");
                char receivedFile[MAX_FILENAME_SIZE];
                memset( receivedFile, 0 , MAX_FILENAME_SIZE );
                read( newfd, receivedFile, MAX_FILENAME_SIZE );
                if( strcmp( receivedFile, "file transmission failed :( " ) == 0 )
                    goto fail;

                printf("=>    received file : %s    --- v\n", receivedFile );

#ifdef DEBUG
                printf("\n[DBG] %s() : line_%d : receivedFile = %s\n", __FUNCTION__, __LINE__, receivedFile );
#endif // DEBUG
//--------------------------------------------------------------------------------------------------------------------
                printf("=>    receving fileSize of encoding table    --- ");

                char integer[4];
                read( newfd, integer, sizeof(int) );
                int fileSize = *(int*)integer;
                puts("v");

#ifdef DEBUG
                printf("\n[DBG] %s() : line_%d : fileSize of table = %d\n", __FUNCTION__, __LINE__, fileSize );
#endif // DEBUG
//--------------------------------------------------------------------------------------------------------------------
                printf("=>    receving encoding table    --- ");

                unsigned char *tableBuf = (unsigned char*)malloc( fileSize );
                memset( tableBuf, 0, fileSize );
                read( newfd, tableBuf, fileSize );
                puts("v");
//--------------------------------------------------------------------------------------------------------------------
                printf("=>    building encoding table    --- ");
                char tableName[strlen(receivedFile) +1 + 6];
                memset( tableName, 0, sizeof(tableName) );
                strcpy( tableName, receivedFile );
                strcat( tableName, "_table" );
                tableName[strlen( tableName )] = '\0';

#ifdef DEBUG
                printf("\n[DBG] %s() : line_%d : tableName = %s, strlen of tableName = %zu\n", __FUNCTION__, __LINE__, tableName, strlen( tableName ) );
#endif // DEBUG

                FILE *table;
                if( (table = fopen( tableName, "w")) == 0 )
                {
                    perror("run_srv() : fopen() : table!!\n");
                    shutdown( newfd, 2 );
                    close(newfd);
                    close(fd);
                    exit(1);
                }
                fwrite( tableBuf, fileSize, 1, table );
                fclose(table);
                free(tableBuf);
                puts("v");

//--------------------------------------------------------------------------------------------------------------------
                printf("=>    creating %s.result    --- ", receivedFile );
                char compressedName[strlen(receivedFile) + 1 + 7];
                memset( compressedName, 0, sizeof(compressedName) );
                strcpy( compressedName, receivedFile );
                strcat( compressedName, ".result" );
                compressedName[strlen( compressedName )] = '\0';

#ifdef DEBUG
                printf("\n[DBG] %s() : line_%d : compressedName = %s, strlen of compressedName = %zu\n", __FUNCTION__, __LINE__, compressedName, strlen( compressedName ) );
#endif // DEBUG

                FILE *compressed;
                if( (compressed = fopen( compressedName, "w")) == 0 )
                {
                    perror("run_srv() : fopen() : compressed!!\n");
                    shutdown( newfd, 2 );
                    close(newfd);
                    close(fd);
                    exit(1);
                }

                int len;
                unsigned char readBuf[MAX_BUF_SIZE];
                while( ( len = recv( newfd, readBuf, MAX_BUF_SIZE, 0 ) ) )
                {
                    fwrite( readBuf, 1, len, compressed );
                    bzero( readBuf, MAX_BUF_SIZE );
                }

                fclose(compressed);
                puts("v");
//--------------------------------------------------------------------------------------------------------------------
                puts("\n+++++++++++++++++++++++++++++++++++++++++++");
                puts("+        starts to uncompressing!!        +");
                puts("+++++++++++++++++++++++++++++++++++++++++++\n");
                if( fUncompression(compressedName) == 0 )
                {
                    puts("succeed in receving and uncompressing file!!");
                }

            }
        }
        else
            puts("server is inpatient :(");
        //close()
fail:
        shutdown( newfd, 2 );
        close(newfd);
        do
        {
            printf("wanna shutdown server ? (y/n) : ");
            scanf("%c", &ctrl );
        }
        while( ctrl != 'y' && ctrl != 'n');
    }
    close(fd);

    return 0;
}

