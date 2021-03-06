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
    struct sockaddr_in cli, temp_sockaddr;
    struct timeval delay_ack = { 1, 00000 };
    int cli_len = sizeof( cli );
    int temp_i, temp_i1;
    int segment_len = 0;
    int base = 0, next_receive = 0, out_of_seq = 0;
    int receive_win_iterator = 0;
    int send_win_iterator = 0;
    int nfds;
    float drop = 0;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    uint16_t payload_len = 0;
    char payload[PAYLOAD_SIZE];
    char segment[PSEUDO_HEADER_LENGTH + HEADER_LENGTH + PAYLOAD_SIZE];
    fd_set rfds;
    fd_set afds;

    srand( time( NULL ) );

    nextseqnum = rand() % 10000 + 1;

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
    puts("=====Start the three-way handshake=====");
//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
//  wait for SYN
    segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );

    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
    {
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + 1;
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
    }

//  reply SYN, ACK
    sleep( 1 );
//      1. set tcp header
    source_port = ( uint16_t )atoi( src_port );
    destination_port = temp_sockaddr.sin_port;
    seq_num = nextseqnum;
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
    printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0012 ), inet_ntoa( cli.sin_addr ), ntohs( cli.sin_port ) );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );

//  wait ACK ( carry with file name )
    segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );

    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
    {
        nextseqnum = *( uint32_t* )( tcp_header + 8 );
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + payload_len;
        puts("~~~~~~~~~~~~~~~~~~~R~~~~~~~~~~~~~~~~~~~~~");
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
        puts("~~~~~~~~~~~~~~~~~~~R~~~~~~~~~~~~~~~~~~~~~");
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
    if( ( file = fopen( buf, "wb" ) ) == 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
    sleep( 5 );
    while( 1 )
    {
        nfds = getdtablesize();
        FD_ZERO( &afds );
        FD_SET( connect_socket, &afds );
        memcpy( &rfds, &afds, sizeof( rfds ) );
        if( select( nfds, &rfds, ( fd_set* ) 0, ( fd_set* ) 0, &delay_ack ) < 0 )
        {
            fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
            perror("");
            exit( 1 );
        }
        if( FD_ISSET( connect_socket, &rfds ) )
        {
            segment_len = recvfrom( connect_socket, segment, sizeof( segment ), 0, ( struct sockaddr* ) &cli, &cli_len );
//            if( *( uint8_t* )( segment + PSEUDO_HEADER_LENGTH + 13 ) == 0x01 )

            if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
            {
                if( flags_type != 0x0001 )
                {
                    if( ( drop = ( float )( rand() % 100 ) / 100 ) < 0.5 )
                    {
                        puts("\nI drop this segment !! ");
                        printf("drop ( %u, %u)\n\n", *(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 4),*(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 8));
                        memset( segment, 0, SEGMENT_SIZE );
                        continue;
                    }
                }

                printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), temp_sockaddr.sin_port );
                seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
                nextseqnum = *( uint32_t* )( tcp_header + 8 );
//                printf("~~~~~~~~~~~~\nnextacknum = %u, next_receive = %d\n~~~~~~~~~~~~~~~~~~~~~~\n", nextacknum, next_receive);
                //後面的封包先送到 seq > expect seq
                if( *( uint32_t* )( tcp_header + 4 ) > nextacknum )
                {
                    for( temp_i1 = 0, temp_i = *( uint32_t* )( tcp_header + 4 ) - nextacknum ; temp_i > 0 ; temp_i -= PAYLOAD_SIZE )
                    {
                        temp_i1++;
                    }
                    //算差幾個 segment
                    if( next_receive + temp_i1 - base < WINDOW_SIZE)
                    {
                        if( receive_window[( next_receive + temp_i1 ) % WINDOW_SIZE] == NULL )
                        {
                            out_of_seq = temp_i1;
                            receive_window[( next_receive + temp_i1 ) % WINDOW_SIZE] = ( char* )malloc( segment_len );
                            memcpy( receive_window[( next_receive + temp_i1 ) % WINDOW_SIZE], segment, segment_len );
                            puts("out of seq");
                            goto outofseq;
                        }
                    }
                    else
                    {
//                        printf("drop2 ( %u, %u)\n\n", *(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 4),*(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 8));
                        memset( segment, 0, SEGMENT_SIZE );
                        continue;
                    }
                }
                else if( *( uint32_t* )( tcp_header + 4 ) < nextacknum )
                {
//                    printf("drop3 ( %u, %u)\n\n", *(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 4),*(uint32_t*)(segment + PSEUDO_HEADER_LENGTH + 8));
                    memset( segment, 0, SEGMENT_SIZE );
                    continue;
                }
                else if( receive_window[next_receive % WINDOW_SIZE] == NULL )
                {

                    receive_window[next_receive % WINDOW_SIZE] = ( char* )malloc( segment_len );
                    memcpy( receive_window[next_receive % WINDOW_SIZE], segment, segment_len );

                }

                // next expected   =            rcv seq               + data length
                nextacknum = *( uint32_t* )( tcp_header + 4 ) + ( uint32_t )payload_len;

                next_receive++;
                //有封包不見
                if( out_of_seq > 0 )
                {
                    for( temp_i = next_receive ; temp_i < next_receive + out_of_seq  ; temp_i++, next_receive++ )
                    {
                        out_of_seq--;
                        if( receive_window[temp_i % WINDOW_SIZE] != NULL )
                        {
                            nextacknum = *( uint32_t* )( receive_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) + *( uint16_t* )( receive_window[temp_i % WINDOW_SIZE] + 10 ) - HEADER_LENGTH;
                        }
                        else
                        {
                            puts("still out of seq");
                            goto outofseq;
                        }
                    }
                }
                if( flags_type == 0x0001 )
                {
                    break;
                }
            }
        }
        else
        {
outofseq:
//            printf("base = %d, next_receive = %d\n", base, next_receive);
            for( temp_i = base; temp_i < next_receive ; temp_i++ )
            {
                if( receive_window[temp_i % WINDOW_SIZE] == NULL )
                    break;
                payload_len = *( uint16_t* )( receive_window[temp_i % WINDOW_SIZE] + 10 ) - HEADER_LENGTH;
                fwrite( receive_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + HEADER_LENGTH, 1, payload_len, file );
                printf("write %u, next_receive = %u\n", temp_i,next_receive);
                free( receive_window[temp_i % WINDOW_SIZE] );
                receive_window[temp_i % WINDOW_SIZE] = NULL;
            }
            base = next_receive;
//            printf("base =  %u, next_receive = %u, out_of_seq = %d\n", base,next_receive, out_of_seq);
            //  reply ACK
//      1. set tcp header
            source_port = ( uint16_t )atoi( src_port );
            destination_port = temp_sockaddr.sin_port;
            if( !FD_ISSET( connect_socket, &rfds ) )
                seq_num = ++nextseqnum;
            else
                seq_num = nextseqnum;
            ack_num = nextacknum;
            data_offset_flags = HEADER_LENGTH;
            data_offset_flags = ( data_offset_flags << 16 ) + 0x0010;
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

            printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0010 ), inet_ntoa( cli.sin_addr ), ntohs( cli.sin_port ) );
            seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );

            sleep( 3 );
        }
    }
    //  reply SYN, ACK
    sleep( 1 );
//      1. set tcp header
    source_port = ( uint16_t )atoi( src_port );
    destination_port = temp_sockaddr.sin_port;
    seq_num = nextseqnum;
    ack_num = nextacknum;
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0011;
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
    printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0011 ), inet_ntoa( cli.sin_addr ), ntohs( cli.sin_port ) );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );

    sleep(1);

//  wait ACK ( carry with file name )
    segment_len = recvfrom( connect_socket, segment, SEGMENT_SIZE, 0, ( struct sockaddr* ) &cli, &cli_len );
    memset( segment, 0 ,SEGMENT_SIZE);

    segment_len = recvfrom( connect_socket, segment, SEGMENT_SIZE, 0, ( struct sockaddr* ) &cli, &cli_len );

    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
    {
        nextseqnum = *( uint32_t* )( tcp_header + 8 );
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + payload_len;

        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( cli.sin_addr ), ntohs( cli.sin_port ) );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );

    }

    puts("=====Complete the three-way handshake=====");

    for( temp_i = 0 ; temp_i < WINDOW_SIZE ; temp_i++ )
        if( receive_window[temp_i] != NULL )
            free( receive_window[temp_i] );

    close( connect_socket );
}

int main( int argc, char* argv[] )
{
    if( argc == 2 )
        run_srv( argv[1] );
    return 0;
}
