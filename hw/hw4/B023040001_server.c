#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cubelib.h"
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa

#define BUF_SIZE 1500

char buf[BUF_SIZE];

char* window[WINDOW_SIZE];

void run_srv( char* src_port )
{
    int temp_i;
    int segment_len = 0;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    uint16_t payload_len = 0;
    char payload[PAYLOAD_SIZE];
    char segment[PSEUDO_HEADER_LENGTH + HEADER_LENGTH + PAYLOAD_SIZE];
    struct sockaddr_in cli, temp_sockaddr;
    int cli_len = sizeof( cli );

    int win_iterator = 0;
    for( temp_i = 0 ; temp_i < WINDOW_SIZE ; temp_i++ )
        window[temp_i] = NULL;
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
    uint16_t destination_port = 0;
//    *( uint16_t* )( tcp_header + 2 ) =  destination_port;
//      set Sequence number
    uint32_t seq_num = 0;
    *( uint32_t* )( tcp_header + 4 ) = seq_num;
//      set Ack number
    uint32_t ack_num = 0;
    *( uint32_t* )( tcp_header + 8 ) = ack_num;
//      set Data offset
    uint16_t header_len = HEADER_LENGTH;
    header_len = header_len << 16;
//      set Flags
    uint16_t flags = 0;
//      set Window size
    uint16_t win_size = 0;
    *( uint16_t* )( tcp_header + 14 ) = win_size;
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
    10~11 (16  bits) : TCP length
    */
//      set Source address
    uint32_t source_addr = 0;
//      set Destination address
    uint32_t destination_addr = 0;
//      set Zeros + Protocol
    uint16_t zeros_protocol = 6;
    *( uint8_t* )( pseudo_header + 8 ) = zeros_protocol;
//      set Tcp length
    uint16_t tcp_len = 0;
//--------------------------------------------------------------------------------------------------------------------
//create socket
    puts("Listening for client...");
    int connect_socket = passiveUDP( src_port );
//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
//  wait for SYN
    segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );
    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, window[win_iterator], &temp_sockaddr, &flags_type ) == 0 )
    {
        win_iterator++;
    }
    puts("=====Start the three-way handshake=====");
    printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
//  reply SYN, ACK
    memset( pseudo_header, 0, PSEUDO_HEADER_LENGTH );
    source_addr = *( uint32_t* )( window[win_iterator - 1] + 4 );
    destination_addr = cli.sin_addr.s_addr;
    zeros_protocol = 6;

    memset( tcp_header, 0, HEADER_LENGTH );

//--------------------------------------------------------------------------------------------------------------------

    recvfrom( connect_socket, buf, BUF_SIZE, 0, ( struct sockaddr* ) &cli, &cli_len );
    printf("\n\nip : %s, port : %d\n", inet_ntoa( cli.sin_addr ) , ntohs( cli.sin_port ) );
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
