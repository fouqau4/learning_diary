#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cubelib.h"
#include <netinet/in.h>
#include <arpa/inet.h> //inet_ntoa
#include <time.h>

#define BUF_SIZE 1500

char buf[BUF_SIZE];

char* receive_window[WINDOW_SIZE];
char* send_window[WINDOW_SIZE];

uint32_t nextseqnum = 0;
uint32_t nextacknum = 0;

void run_srv( char* src_port )
{
    int temp_i;
    int segment_len = 0;
    int base = 0, next_receive = 0;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    uint16_t payload_len = 0;
    char payload[PAYLOAD_SIZE];
    char segment[PSEUDO_HEADER_LENGTH + HEADER_LENGTH + PAYLOAD_SIZE];
    struct sockaddr_in cli, temp_sockaddr;
    int cli_len = sizeof( cli );
    srand( time( NULL ) );

    int receive_win_iterator = 0;
    int send_win_iterator = 0;
    for( temp_i = 0 ; temp_i < WINDOW_SIZE ; temp_i++ )
        receive_window[temp_i] = NULL, send_window[temp_i] = NULL;
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
    uint16_t source_port = 0;
//      set Destination port
    uint16_t destination_port = 0;
//      set Sequence number
    uint32_t seq_num = 0;
//      set Ack number
    uint32_t ack_num = 0;
//      set Data offset + Flags
    uint16_t data_offset_flags = 0;
//      set Window size
    uint16_t win_size = 0;
//      set Checksum
    uint16_t checksum = 0;
//      set Urgent pointer
    uint16_t ugn_ptr = 0;

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
    puts("=====Start the three-way handshake=====");
    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
    {
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + 1;
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
    }

//  reply SYN, ACK
//      1. set tcp header
    source_port = ( uint16_t )atoi( src_port );
    destination_port = temp_sockaddr.sin_port;
    seq_num = rand() % 10000 + 1;
    ack_num = nextacknum;
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0012;
    win_size = WINDOW_SIZE;
    checksum = 0;
    ugn_ptr = 0;

    set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );

//      2. set payload
    memset( payload, 0, PAYLOAD_SIZE );

//      3.set pseudo header
    source_addr = *( uint32_t* )( pseudo_header + 4 );
    destination_addr = cli.sin_addr.s_addr;
    zeros_protocol = 6;
    payload_len = 0;
    tcp_len = HEADER_LENGTH + payload_len;

    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );

//      4. build segment
    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = cumulate_checksum( segment, segment_len );

//      5. send

    sendto( connect_socket, segment, segment_len, 0, ( struct sockaddr* ) &cli, cli_len );
    printf("Send a packet(%s) to %s : %hu\t-----  ", identify_flags( 0x0012 ), inet_ntoa( cli.sin_addr ), ntohs( cli.sin_port ) );

//  wait ACK ( carry with file name )
    segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );
    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, &receive_window[next_receive % 20], &temp_sockaddr, &flags_type ) == 0 )
    {
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + payload_len;
        next_receive++;
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
    }

    puts("=====Complete the three-way handshake=====");
//--------------------------------------------------------------------------------------------------------------------
//  receive file

//  create file
    printf("filename = -- %s --\n", payload );
    memset( buf, 0, BUF_SIZE );
    strcpy( buf, "rcv_" );
    strcat( buf, payload );
    FILE *file;
    if( ( file = fopen( buf, "w" ) ) == 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }

    while( 1 )
    {
        segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );
        if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, &receive_window[next_receive % 20], &temp_sockaddr, &flags_type ) == 0 )
        {
            nextacknum = *( uint32_t* )( tcp_header + 4 ) + payload_len;
            next_receive++;
            printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
            seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
        }
    }


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
