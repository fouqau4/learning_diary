#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "cubelib.h"

#define BUF_SIZE 1500

char buf[BUF_SIZE];

void run_cli( char* dest_ip, char* dest_port, char* src_port )
{int temp_i;
    uint32_t temp_ui32t;
    uint16_t payload_len = 0;
    char payload[PAYLOAD_SIZE];
    char segment[SEGMENT_SIZE];
//--------------------------------------------------------------------------------------------------------------------
//Header
//  1. Tcp header
    char tcp_header[HEADER_LENGTH];
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
//      set Source port
    uint16_t source_port = ( uint16_t )atoi( src_port );
    *( uint16_t* )tcp_header = source_port;
//      set Destination port
    uint16_t destination_port = ( uint16_t )atoi( dest_port );
    *( uint16_t* )( tcp_header + 2 ) = destination_port ;
    printf("dest port = %hd\n", *( uint16_t* )( tcp_header + 2 ) );
//      set Sequence number
    uint32_t seq_num = 0;
    *( uint32_t* )( tcp_header + 4 ) = seq_num ;
//      set Ack number
    uint32_t ack_num = 0;
    *( uint32_t* )( tcp_header + 8 ) = ack_num ;
//      set Data offset
    uint16_t header_len = HEADER_LENGTH;
    header_len = header_len << 16;
//      set Flags
    uint16_t flags = 0;
//      set Window size
    uint16_t win_size = 0;
    *( uint16_t* )( tcp_header + 14 ) =  win_size;
//      set Checksum
    uint16_t checksum = 0;
    *( uint16_t* )( tcp_header + 16 ) = 0;
//      set Urgent pointer
    uint16_t ugn_ptr = 0;
    *( uint16_t* )( tcp_header + 18 ) = ugn_ptr;
//  ---------------------
//  2. pseudo header
    char pseudo_header[PSEUDO_HEADER_LENGTH];
    /*
    0~3   (32  bits) : Source address
    4~7   (32  bits) : Destination address
    8     (8   bits) : Zeros
    9     (8   bits) : Protocol
    10~11 (16  bits) : TCP length ( tcp header + payload )
    */
//      set Source address
    uint32_t source_addr = 0;
//      set Destination address
    uint32_t destination_addr = 0;
//      set Zeros + Protocol
    uint16_t zeros_protocol = 6;
//      set Tcp length
    uint16_t tcp_len = 0;
//--------------------------------------------------------------------------------------------------------------------
//create socket
//  create destination socket
    int dest_socket;
    struct sockaddr_in dest;
    dest.sin_addr.s_addr = inet_addr( dest_ip );
    dest.sin_family = AF_INET;
    dest.sin_port = htons( ( unsigned short )atoi( dest_port ) );

    destination_addr = dest.sin_addr.s_addr;

    if( ( dest_socket = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//  bind source port with socket
    struct sockaddr_in src;
    int src_len = sizeof( src );
    src.sin_addr.s_addr = htonl( INADDR_ANY );
    src.sin_port = htons( ( uint16_t )atoi( src_port ) );
    src.sin_family = AF_INET;

    source_addr = src.sin_addr.s_addr;

    if( bind( dest_socket, ( struct sockaddr* ) &src, src_len ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
    flags = 2;
    *( uint16_t* )( tcp_header + 12 ) = header_len + flags;
    memset( payload, 0, PAYLOAD_SIZE );
    strcpy( payload, "HI motherfucker" );
    payload_len = strlen( payload );
    tcp_len = HEADER_LENGTH + payload_len;
    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );

    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    checksum = cumulate_checksum( segment, tcp_len + 12 );
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = checksum;
    printf("checksum = %x\n", checksum );
    *( uint16_t* )( tcp_header + 16 ) = checksum;


//  send SYN
    sendto( dest_socket, segment, PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len, 0, ( struct sockaddr* ) &dest, sizeof( dest ) );


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



//    uint16_t a = 0x02ff;uint32_t b = a + 0xffff;
//
//    printf("%x\n", ( a & 0xff00 ) );

    return 0;
}
