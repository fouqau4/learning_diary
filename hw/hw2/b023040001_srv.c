#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b023040001_srv.h"
#define MAX_BUF_SIZE 1024
#define MAX_FILENAME_SIZE 1024
//IPPROTO_TCP is defined in <netinet/in.h>
int run_srv()
{
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //bind()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(8787);
    srv.sin_addr.s_addr = htonl( INADDR_ANY );



    if(bind( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
    {
        perror("bind error!!\n");
        close(fd);
        exit(1);
    }
    //listen()
    if(listen( fd, 5) < 0)
    {
        perror("listen error!!\n");
        close(fd);
        exit(1);
    }
    //select()

    fd_set readfds;
    struct timeval waitCli = {100,0};

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    if(select( fd+1, &readfds, 0, 0, &waitCli ) < 0)
    {
        perror("select error!!\n");
        close(fd);
        exit(1);
    }

    //accept()
    int newfd;
    struct sockaddr_in cli;
    int cli_len = (int)sizeof(cli);
    unsigned char buf[MAX_BUF_SIZE]; memset( buf, 0, sizeof(buf) );

    if(FD_ISSET( fd, &readfds ))
    {
        puts("fd free!!");
        if((newfd = accept( fd, (struct sockaddr*) &cli, &cli_len))< 0)
        {
            perror("accept error!!\n");
            close(fd);
            exit(1);
        }
    //read()
        else
        {
            char receivedFile[MAX_FILENAME_SIZE]; memset( receivedFile, 0 , sizeof(receivedFile) );
            read( newfd, receivedFile, sizeof(receivedFile) );
            puts("received file :");
            puts(receivedFile);

            FILE *table;
            char tableName[strlen(receivedFile) +1 + 6]; memset( tableName, 0, sizeof(tableName) );
            strcpy( tableName, receivedFile );
            strcat( tableName, "_table" );

            if( (table = fopen( tableName, "w")) == 0 )
            {
                perror("run_srv() : fopen() : table!!\n");
                close(newfd);
                close(fd);
                exit(1);
            }
            long int fileSize;
            puts("receving fileSize!!\n");
            read( fd, buf, sizeof(buf) );
            fileSize = atol(buf);
            printf("fileSize = %ld\n", fileSize);

            unsigned char *tableBuf = (unsigned char*)malloc(fileSize); memset( tableBuf, 0, fileSize );
            read( fd, tableBuf, fileSize );
            puts("succeed in receving table\n");
            fwrite( tableBuf, fileSize, 1, table );
            fclose(table);
            puts("succeed in building tableFile\n");

            FILE *compressed;
            char compressedName[strlen(receivedFile) + 1 + 7]; memset( compressedName, 0, sizeof(compressedName) );
            strcpy( compressedName, receivedFile );
            strcat( compressedName, ".result" );
            if( (compressed = fopen( compressedName, "w")) == 0 )
            {
                perror("run_srv() : fopen() : compressed!!\n");
                close(newfd);
                close(fd);
                exit(1);
            }

            memset( buf, 0, sizeof(buf) );
            read( newfd, buf, sizeof(buf) );
            fileSize = atol(buf);
            unsigned char *readBuf = (unsigned char*)malloc( fileSize ); memset( readBuf, 0, sizeof(readBuf) );
            read( newfd, readBuf, fileSize );
            fwrite( readBuf, fileSize, 1, compressed );
            fclose(compressed);

            free(tableBuf);
            free(readBuf);

            if( fUncomperssion(compressedName) == 0 )
            {
                puts("succeed in receving and uncompressing file!!");
            }

        }
    }
    else
        puts("server is inpatient :(");

    //close()
    close(newfd);
    close(fd);
    return 0;
}

