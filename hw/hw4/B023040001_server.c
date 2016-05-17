#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cubelib.h"
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa

#define BUF_SIZE 1024

char buf[BUF_SIZE];

void run_srv( char* port_num )
{
char tcp_header[20];
    /*
    0~1   (16 bits) : Source port
    2~3   (16 bits) : Destination port
    4~7   (32 bits) : Sequence number
    8~11  (32 bits) : Ack number
    12~13 (4  bits) : Data offset ( header length )
          (3  bits) : Reserved
          (9  bits) : Flags( Ns|CWR|ECE|URG|ACK|PSH|RST|SYN|FIN )
    14~15 (16 bits) : Window size
    16~17 (16 bits) : Checksum
    18~19 (16 bits) : Urgent pointer
    */
//--------------------------------------------------------------------------------------------------------------------
//create socket
    int connect_socket = passiveUDP( port_num );
//--------------------------------------------------------------------------------------------------------------------
    struct sockaddr_in cli;
    int cli_len = sizeof( cli );
    recvfrom( connect_socket, buf, BUF_SIZE, 0, ( struct sockaddr* ) &cli, &cli_len );
    printf("ip : %s, port : %d\n", inet_ntoa( cli.sin_addr ) , ntohs( cli.sin_port ) );
    puts(buf);
    recvfrom( connect_socket, buf, BUF_SIZE, 0, ( struct sockaddr* ) &cli, &cli_len );
    printf("ip : %s, port : %d\n", inet_ntoa( cli.sin_addr ) , ntohs( cli.sin_port ) );
    puts(buf);
    puts("RECEIVE!!");
    close( connect_socket );
}

int main( int argc, char* argv[] )
{
    if( argc == 2 )
        run_srv( argv[1] );
    return 0;
}
