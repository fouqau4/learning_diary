#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "b023040001_cli.h"
#include "b023040001_huffman.h"
#define MAX_BUF_SIZE 1024

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
        puts("connect success!!");
        puts("please enter the file you want to send :");
        scanf("%s",buf);
        if( fCompression(buf) == 0 )
        {
            write( fd, buf, sizeof(buf) );
            scanf("%s",buf);
            write( fd, buf, sizeof(buf) );
//            FILE *compressed;
//            if( (compressed = fopen( buf, "r" )) == 0 )
//            {
//                perror("run_cli() : fopen() : compressed!!\n");
//                close(fd);
//                return 1;
//            }
//            fseek( compressed, 0, SEEK_END );
//            long int compressedSize = ftell(compressed);
//            rewind(compressed);
//            unsigned char *writeBuf = (unsigned char*)malloc( compressedSize ); memset( writeBuf, 0, compressedSize );
//            fread( writeBuf, compressedSize, 1, compressed );
//            fclose(compressed);
//            write( fd, writeBuf, compressedSize );
        }
        close(fd);
    }

    return 0;
}
