#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <endian.h>
#define BUF_SIZE 1500

char buf[BUF_SIZE];

void run_cli( char* dest_ip, char* dest_port, char* src_port )
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
//  set Source port
    *( uint16_t* )tcp_header = ( uint16_t )atoi( src_port );
//  set Destination port
    *( uint16_t* )tcp_header + 2 = ( uint16_t )atoi( dest_port );
//--------------------------------------------------------------------------------------------------------------------
//create socket
//  create destination socket
    int dest_socket;
    struct sockaddr_in dest;
    dest.sin_addr.s_addr = inet_addr( dest_ip );
    dest.sin_family = AF_INET;
    dest.sin_port = htons( ( unsigned short )atoi( dest_port ) );

    if( ( dest_socket = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//  bind source port with socket
    struct sockaddr_in cli;
    int cli_len = sizeof( cli );
    cli.sin_addr.s_addr = htonl( INADDR_ANY );
    cli.sin_port = htons( ( uint16_t )atoi( src_port ) );
    cli.sin_family = AF_INET;

    if( bind( dest_socket, ( struct sockaddr* ) &cli, cli_len ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
    uint32_t seq_num = 0;
//--------------------------------------------------------------------------------------------------------------------
//choose the file
    FILE *file;
    printf("enter the filename : ");
    memset( buf, 0, BUF_SIZE );
    scanf(" %s", buf );
    buf[strlen( buf )] = '\0';
    if( ( file = fopen( buf, "rb" ) ) == 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//--------------------------------------------------------------------------------------------------------------------
//file split
    fclose( file );
//--------------------------------------------------------------------------------------------------------------------
    strcpy( buf, "HI mother fucker" );
    sendto( dest_socket, buf, strlen( buf ), 0, ( struct sockaddr* ) &dest, sizeof( dest ) );
    puts("SENDED!!");
    strcpy( buf, "YOYOYOHAHAHA" );
    sendto( dest_socket, buf, strlen( buf ), 0, ( struct sockaddr* ) &dest, sizeof( dest ) );
    close( dest_socket );
}
int main( int argc, char* argv[] )
{
    if( argc == 4 )
        run_cli( argv[1], argv[2], argv[3] );

    char a = (char)(00100000);
    printf("%d", (int)a);

    return 0;
}
