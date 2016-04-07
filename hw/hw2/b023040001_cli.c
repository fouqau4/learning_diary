#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "b023040001_cli.h"
#include "b023040001_huffman.h"
#define MAX_BUF_SIZE 1024
#define MAX_FILENAME_SIZE 1024

int run_cli(char *srvIp, int port ){
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //connect()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(srvIp);

    unsigned char buf[MAX_BUF_SIZE]; memset( buf, 0, sizeof(buf) );

    if(srv.sin_addr.s_addr == (in_addr_t)-1)
        perror("inet_addr failed\n"), exit(1);

    if(connect( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("connect error!!"), exit(1);
    //write()
    else
    {
        char sendedFile[MAX_FILENAME_SIZE]; memset( sendedFile, 0, sizeof(sendedFile) );
        puts("connect success!!");
        puts("please enter the file you want to send :");
        scanf("%s", sendedFile);
        write( fd, sendedFile, strlen(sendedFile) );

        if( fCompression(sendedFile) == 0 )
        {
//            scanf("%s",buf);
//            write( fd, buf, sizeof(buf) );
            char tableName[strlen(sendedFile) + 1 + 6]; memset( tableName, 0 ,sizeof(tableName) );
            strcpy( tableName, sendedFile );
            strcat( tableName, "_table" );
            FILE *table;
            if( (table = fopen( tableName, "r" )) == 0 )
            {
                perror("run_cli() : fopen() : table!!\n");
                close(fd);
                return 1;
            }

            fseek( table, 0, SEEK_END );
            long int fileSize = ftell(table);
            rewind(table);
            unsigned char *tableBuf = (unsigned char*)malloc( fileSize ); memset( tableBuf, 0, fileSize );
            fread( tableBuf, fileSize, 1, table );
            write( fd, fileSize, sizeof(fileSize) );
            write( fd, tableBuf, fileSize );
            puts("succeed in sending table!!");

            char compressedName[strlen(sendedFile) + 1 + 7]; memset( compressedName, 0, sizeof(compressedName) );
            strcpy( compressedName, sendedFile );
            strcat( compressedName, ".result" );
            FILE *compressed;
            if( (compressed = fopen( compressedName, "r" )) == 0 )
            {
                perror("run_cli() : fopen() : compressed!!\n");
                close(fd);
                return 1;
            }

            fseek( compressed, 0, SEEK_END );
            fileSize = ftell(compressed);
            rewind(compressed);
            unsigned char *writeBuf = (unsigned char*)realloc( writeBuf, fileSize ); memset( writeBuf, 0, fileSize );

            write( fd, &fileSize, sizeof(fileSize) );
            fread( writeBuf, fileSize, 1, compressed );
            fclose(compressed);
            write( fd, writeBuf, fileSize );

            free(writeBuf);
        }
        else
            puts("file transmission failed");
        close(fd);
    }

    return 0;
}
