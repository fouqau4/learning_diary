#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "cubelib.h"
#include <time.h>

#define BUF_SIZE 1500

char* receive_window[WINDOW_SIZE];
char* send_window[WINDOW_SIZE];

char buf[BUF_SIZE];

void run_cli( char* dest_ip, char* dest_port, char* src_port )
{
    int temp_i;
    int segment_len = 0;
    int receive_win_iterator = 0;
    uint16_t payload_len = 0;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    char payload[PAYLOAD_SIZE];
    char segment[SEGMENT_SIZE];
    struct sockaddr_in temp_sockaddr;
    srand( time( NULL ) );

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
    uint16_t win_size = WINDOW_SIZE;
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
    int dest_len = sizeof( dest );

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

    if( bind( dest_socket, ( struct sockaddr* ) &src, src_len ) < 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
//    memset( payload, 0, PAYLOAD_SIZE );
//    strcpy( payload, "HI motherfucker" );
    puts("=====Start the three-way handshake=====");

//  send SYN
//      1.set pseudo header
    source_addr = src.sin_addr.s_addr;
    destination_addr = dest.sin_addr.s_addr;
    zeros_protocol = 6;
    payload_len = 0;
    tcp_len = HEADER_LENGTH + payload_len;

    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );
//      2. set tcp header
    source_port = ( uint16_t )atoi( src_port );
    destination_port = ( uint16_t )atoi( dest_port );
    seq_num = rand() % 10000 + 1;
    ack_num = 0;
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0002;
    win_size = WINDOW_SIZE;
    checksum = 0;
    ugn_ptr = 0;

    set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );
//      3. set payload
    memset( payload, 0, PAYLOAD_SIZE );
//      4. build segment
    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
    checksum = cumulate_checksum( segment, tcp_len + 12 );
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = checksum;
//      5. send
    sendto( dest_socket, segment, PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len, 0, ( struct sockaddr* ) &dest, dest_len );
    printf("Send a packet(%s) to %s : %hu\n", identify_flags( 0x0002 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
//  wait SYN/ACK
    memset( segment, 0, SEGMENT_SIZE );
    segment_len = recvfrom( dest_socket, segment, SEGMENT_SIZE, 0, ( struct sockaddr* ) &dest, &dest_len );
    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, &receive_window[receive_win_iterator], &temp_sockaddr, &flags_type ) == 0 )
    {
        receive_win_iterator++;
    }
    printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( dest.sin_addr ), temp_sockaddr.sin_port );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );

//  send ACK

//      1.set pseudo header
    source_addr = *( uint32_t* )( receive_window[receive_win_iterator - 1] + 4 );
    destination_addr = dest.sin_addr.s_addr;
    zeros_protocol = 6;
    payload_len = 0;
    tcp_len = HEADER_LENGTH + payload_len;
    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );

//      2. set tcp header
    source_port = ( uint16_t )atoi( dest_port );
    destination_port = temp_sockaddr.sin_port;
    seq_num = *( uint32_t* )( receive_window[receive_win_iterator - 1] + PSEUDO_HEADER_LENGTH + 8 ) + 1;
    ack_num = *( uint32_t* )( receive_window[receive_win_iterator - 1] + PSEUDO_HEADER_LENGTH + 4 ) + 1;
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0010;
    win_size = WINDOW_SIZE;
    checksum = 0;
    ugn_ptr = 0;

    set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );

//      3. set payload
    memset( payload, 0, PAYLOAD_SIZE );

//      4. build segment
    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = cumulate_checksum( segment, segment_len );

//      5. send
    sendto( dest_socket, segment, segment_len, 0, ( struct sockaddr* ) &dest, dest_len );

    printf("Send a packet(%s) to %s : %hu\n", identify_flags( 0x0010 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
    puts("=====Complete the three-way handshake=====");
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
