#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "b023040001_cli.h"
#include "b023040001_huffman.h"
#define MAX_FILENAME_SIZE 1024
//#define DEBUG


int run_cli(char* srvIp, char* port ){
    //socket()
    int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //connect()
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port = htons( (u_short)atoi( port ) );
    srv.sin_addr.s_addr = inet_addr(srvIp);

    if(srv.sin_addr.s_addr == (in_addr_t)-1)
        perror("inet_addr failed\n"), exit(1);

    if(connect( fd, (struct sockaddr*) &srv, sizeof(srv) ) < 0)
        perror("connect error!!"), exit(1);
    //write()
    else
    {
        puts("\n+++++++++++++++++++++++++++++++++++++++++++");
        puts("+             connect successfully!!      +");
        puts("+++++++++++++++++++++++++++++++++++++++++++\n");
        printf("=>    please enter the file you want to send : \n=>    ");

        char sendedFile[MAX_FILENAME_SIZE]; memset( sendedFile, 0, sizeof(sendedFile) );
        scanf("%s", sendedFile);
        sendedFile[strlen( sendedFile )] = '\0';

        #ifdef DEBUG
        printf("\n[DBG] %s() : line_%d : sendedFile = %s, strlen of sendedFile = %zu\n", __FUNCTION__, __LINE__, sendedFile, strlen( sendedFile ) );
        #endif // DEBUG


        puts("\n+++++++++++++++++++++++++++++++++++++++++++");
        puts("+          starts to compressing!!        +");
        puts("+++++++++++++++++++++++++++++++++++++++++++\n");
        if( fCompression(sendedFile) == 0 )
        {
            write( fd, sendedFile, strlen( sendedFile ) );
            puts("\n∥=========================================∥");
            puts("∥      succeed in compressing file!!!     ∥");
            puts("∥=========================================∥\n");
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    fetching encoding table    --- ");
            char tableName[strlen(sendedFile) + 1 + 6]; memset( tableName, 0 , sizeof(tableName) );
            strcpy( tableName, sendedFile );
            strcat( tableName, "_table" );
            tableName[strlen( tableName )] = '\0';

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : tableName = %s, strlen of tableName = %zu\n", __FUNCTION__, __LINE__, tableName, strlen( tableName ) );
            #endif // DEBUG

            FILE *table;
            if( (table = fopen( "myTcp.h_table", "r" )) == 0 )
            {
                perror("run_cli() : fopen() : table!!\n");
                shutdown( fd, 2 );
                close(fd);
                return 1;
            }

            int fileSize;
            fseek( table, 0, SEEK_END );
            fileSize = ftell(table);
            rewind(table);

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : fileSize of table = %d\n", __FUNCTION__, __LINE__, fileSize );
            #endif // DEBUG

            unsigned char *tableBuf = (unsigned char*)malloc( fileSize ); memset( tableBuf, 0, fileSize );
            fread( tableBuf, fileSize, 1, table );
            fclose( table );
            puts("v");

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : elements in tableBuf\n", __FUNCTION__, __LINE__ );
            int Dtemp_i;
            for( Dtemp_i = 0 ; Dtemp_i < fileSize ; Dtemp_i++ )
                printf("    [DBG] No.%4d : %d\n", Dtemp_i, (int)tableBuf[Dtemp_i] );
            #endif // DEBUG
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    sending fileSize of encoding table    --- ");

            char integer[4];
            *(int*)integer = fileSize;

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : *(int*)integer = %d\n", __FUNCTION__, __LINE__, *(int*)integer );
            #endif // DEBUG

            write( fd, integer, sizeof(int) );
            puts("v");
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    sending encoding table    --- ");
            write( fd, tableBuf, fileSize );
            free( tableBuf );
            puts("v");
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    fetching %s.result    --- ", sendedFile );
            char compressedName[strlen(sendedFile) + 1 + 7]; memset( compressedName, 0, sizeof(compressedName) );
            strcpy( compressedName, sendedFile );
            strcat( compressedName, ".result" );
            compressedName[strlen( compressedName )] = '\0';

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : compressedName = %s, strlen of compressedName = %zu\n", __FUNCTION__, __LINE__, compressedName, strlen( compressedName ) );
            #endif // DEBUG

            FILE *compressed;
            if( (compressed = fopen( compressedName, "r" )) == 0 )
            {
                perror("run_cli() : fopen() : compressed!!\n");
                shutdown( fd, 2 );
                close(fd);
                return 1;
            }

            fseek( compressed, 0, SEEK_END );
            fileSize = ftell(compressed);
            rewind(compressed);

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : fileSize of compressed = %d\n", __FUNCTION__, __LINE__, fileSize );
            #endif // DEBUG


            *(int*)integer = fileSize;

            #ifdef DEBUG
            printf("\n[DBG] %s() : line_%d : *(int*)integer = %d\n", __FUNCTION__, __LINE__, *(int*)integer );
            #endif // DEBUGř

            unsigned char *writeBuf = (unsigned char*)malloc( fileSize ); memset( writeBuf, 0, fileSize );
            fread( writeBuf, fileSize, 1, compressed );
            fclose(compressed);
            puts("v");
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    sending fileSize of %s.result    --- ", sendedFile );
            write( fd, integer, sizeof(int) );
            puts("v");
//--------------------------------------------------------------------------------------------------------------------
            printf("=>    sending %s.result    --- ", sendedFile );
            write( fd, writeBuf, fileSize );
            puts("v");

            free(writeBuf);
        }
        else
        {
            puts("file transmission failed :( ");
            char err[29] = "file transmission failed :( ";
            err[28] = '\0';
            write( fd, err, sizeof( err ) );
        }
        shutdown( fd, 2 );
        close(fd);
    }

    return 0;
}
