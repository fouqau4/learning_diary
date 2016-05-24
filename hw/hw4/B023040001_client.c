#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "cubelib.h"
#include <time.h>
#include <pthread.h>

#define BUF_SIZE 1500

pthread_mutex_t lock_send_window;
pthread_mutex_t lock_nextseqacknum;
pthread_mutex_t lock_win_iterator;


char* receive_window[WINDOW_SIZE];
char* send_window[WINDOW_SIZE];

char buf[BUF_SIZE];

int dest_socket;
int last_acked = 0, next_send = 0;
uint32_t nextseqnum = 0;
uint32_t nextacknum = 0;

char rcv_segment[SEGMENT_SIZE];
char rcv_tcp_header[HEADER_LENGTH];
char rcv_pseudo_header[PSEUDO_HEADER_LENGTH];
char rcv_payload[PAYLOAD_SIZE];
uint16_t rcv_payload_len;
int rcv_segment_len;
int rcv_dest_len;
uint16_t rcv_flags_type;
uint32_t rcv_temp_ui32t;
struct sockaddr_in rcv_dest;
struct sockaddr_in rcv_temp_sockaddr;

int is_rcv = 0;

void receive_segment()
{

    uint16_t payload_len;
    int segment_len;
    int dest_len;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    struct sockaddr_in dest;
    struct sockaddr_in temp_sockaddr;
    int temp_i;

    while( 1 )
    {
        memset( rcv_segment, 0, SEGMENT_SIZE );
        rcv_segment_len = recvfrom( dest_socket, rcv_segment, SEGMENT_SIZE, 0, ( struct sockaddr* ) &rcv_dest, &rcv_dest_len );
        //------critical section-------
        pthread_mutex_lock( &lock_nextseqacknum );
        if( is_rcv == -1 )
        {
            puts("THREAD EXIT");
            pthread_exit( NULL );
        }
        if( disassemble_segment( rcv_segment, rcv_segment_len, rcv_pseudo_header, rcv_tcp_header, rcv_payload, &rcv_payload_len, NULL, &rcv_temp_sockaddr, &rcv_flags_type ) == 0 )
        {
            is_rcv = 1;
            seq_ack_num_info( *( uint32_t* )( rcv_tcp_header + 4 ), *( uint32_t* )( rcv_tcp_header + 8 ), 0 );
        }
        pthread_mutex_unlock( &lock_nextseqacknum );
        //------------------------------
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( rcv_flags_type ), inet_ntoa( dest.sin_addr ), rcv_temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( rcv_tcp_header + 4 ), *( uint32_t* )( rcv_tcp_header + 8 ), 0 );

    }
}

void run_cli( char* dest_ip, char* dest_port, char* src_port )
{
    int temp_i;
    int segment_len = 0;
    uint16_t payload_len = 0;
    uint16_t flags_type;
    uint32_t temp_ui32t;
    char payload[PAYLOAD_SIZE];
    char segment[SEGMENT_SIZE];
    struct sockaddr_in temp_sockaddr;
    pthread_t receive_ack;

    srand( time( NULL ) );

    pthread_mutex_init( &lock_send_window, PTHREAD_MUTEX_TIMED_NP );
    pthread_mutex_init( &lock_nextseqacknum, PTHREAD_MUTEX_TIMED_NP );
    pthread_mutex_init( &lock_win_iterator, PTHREAD_MUTEX_TIMED_NP );

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
//choose the file
    FILE *file;
    printf("enter the filename : ");
    memset( buf, 0, BUF_SIZE );
    scanf(" %s", buf );

//--------------------------------------------------------------------------------------------------------------------
//3-way handshack -- SYN
//    memset( payload, 0, PAYLOAD_SIZE );
//    strcpy( payload, "HI motherfucker" );
    puts("=====Start the three-way handshake=====");
    sleep( 1 );
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
    seq_num = nextseqnum;
    ack_num = nextacknum;
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
    sendto( dest_socket, segment, segment_len, 0, ( struct sockaddr* ) &dest, dest_len );
    printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0002 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );

//  wait SYN/ACK
    sleep( 1 );
    memset( segment, 0, SEGMENT_SIZE );
    segment_len = recvfrom( dest_socket, segment, SEGMENT_SIZE, 0, ( struct sockaddr* ) &dest, &dest_len );
    if( disassemble_segment( segment, segment_len, pseudo_header, tcp_header, payload, &payload_len, NULL, &temp_sockaddr, &flags_type ) == 0 )
    {
        //------critical section-------
        pthread_mutex_lock( &lock_nextseqacknum );
        nextseqnum++;
        nextacknum = *( uint32_t* )( tcp_header + 4 ) + 1;
        pthread_mutex_unlock( &lock_nextseqacknum );
        //-----------------------------
        printf("Receive a packet(%s) from %s : %hu\n", identify_flags( flags_type ), inet_ntoa( dest.sin_addr ), temp_sockaddr.sin_port );
        seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 0 );
    }

    pthread_create( &receive_ack, 0, (void*)receive_segment, NULL );

//  send ACK ( carry with file name )
    sleep( 1 );
//      1. set tcp header
    source_port = ( uint16_t )atoi( dest_port );
    destination_port = temp_sockaddr.sin_port;
    //------critical section-------
    pthread_mutex_lock( &lock_nextseqacknum );
    seq_num = nextseqnum;
    ack_num = nextacknum;
    pthread_mutex_unlock( &lock_nextseqacknum );
    //-----------------------------
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0010;
    win_size = WINDOW_SIZE;
    checksum = 0;
    ugn_ptr = 0;

    set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );

//      2. set payload
    memset( payload, 0, PAYLOAD_SIZE );
    strcpy( payload, buf );
    payload_len = strlen( payload );

//      3.set pseudo header
    source_addr = src.sin_addr.s_addr;
    destination_addr = dest.sin_addr.s_addr;
    zeros_protocol = 6;
    tcp_len = HEADER_LENGTH + payload_len;

    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );


//      4. build segment
    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = cumulate_checksum( segment, segment_len );

//      5. send
    //------critical section-------
    pthread_mutex_lock( &lock_win_iterator );
    if( send_window[next_send % WINDOW_SIZE] == NULL )
    {
        //------critical section-------
        pthread_mutex_lock( &lock_send_window );
        send_window[next_send % WINDOW_SIZE] = ( char* )malloc( segment_len );
        memcpy( send_window[next_send % WINDOW_SIZE], segment, segment_len );
        pthread_mutex_unlock( &lock_send_window );
        //-----------------------------
        sendto( dest_socket, send_window[next_send % WINDOW_SIZE], segment_len, 0, ( struct sockaddr* ) &dest, dest_len );
        next_send++;
        //------critical section-------
        pthread_mutex_lock( &lock_nextseqacknum );
        nextseqnum += payload_len;
        pthread_mutex_unlock( &lock_nextseqacknum );
        //-----------------------------
    }
    pthread_mutex_unlock( &lock_win_iterator );
    //-----------------------------

    printf("Send a packet(%s) to %s : %hu\n", identify_flags( 0x0010 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );
    puts("=====Complete the three-way handshake=====");
//--------------------------------------------------------------------------------------------------------------------


    if( ( file = fopen( buf, "rb" ) ) == 0 )
    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }

//  send file
    while( !feof( file ) )
    {
        //------critical section-------
        pthread_mutex_lock( &lock_nextseqacknum );
        if( is_rcv )
        {
            is_rcv = 0;
            for( temp_i = last_acked ; temp_i < next_send ; temp_i++ )
            {
                if( send_window[temp_i % WINDOW_SIZE] != NULL )
                {
                    if( *( uint32_t* )( rcv_tcp_header + 8 ) > *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) )
                    {
                        printf("free : %d, seqnum = %u\n", temp_i, *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) );
                        free( send_window[temp_i % WINDOW_SIZE] );
                        send_window[temp_i % WINDOW_SIZE] = NULL;
                        last_acked = temp_i + 1;
                    }
                    else if( *( uint32_t* )( rcv_tcp_header + 8 ) == *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) )
                    {
//      5. send
                        sendto( dest_socket, send_window[temp_i % WINDOW_SIZE], segment_len, 0, ( struct sockaddr* ) &dest, dest_len );

                        printf("\n!!! Resend a packet(%s) to %s : %hu\t\n\n", identify_flags( 0x0010 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
                        seq_ack_num_info( *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ), *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 8 ), 1 );
                    }
                }
            }
        }
        pthread_mutex_unlock( &lock_nextseqacknum );
        //------------------------------

//        printf("last acked : %hu, next send : %d, next seq num : %u\n", last_acked, next_send, nextseqnum );
        if( next_send < last_acked + WINDOW_SIZE )
        {
            if( send_window[next_send % WINDOW_SIZE] == NULL )
            {

//      1. set payload
                memset( payload, 0, PAYLOAD_SIZE );
                payload_len = fread( payload, 1, PAYLOAD_SIZE, file );

//      2. set tcp header
                source_port = ( uint16_t )atoi( src_port );
                destination_port = ( uint16_t )atoi( dest_port );
                seq_num = nextseqnum;
                ack_num = nextacknum;
                data_offset_flags = HEADER_LENGTH;
                data_offset_flags = ( data_offset_flags << 16 ) + 0x0010;
                win_size = WINDOW_SIZE;
                checksum = 0;
                ugn_ptr = 0;

                set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );

//      3.set pseudo header
                source_addr = src.sin_addr.s_addr;
                destination_addr = dest.sin_addr.s_addr;
                zeros_protocol = 6;
                tcp_len = HEADER_LENGTH + payload_len;

                set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );

//      4. build segment
                build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
                segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
                checksum = cumulate_checksum( segment, tcp_len + 12 );
                *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = checksum;
//      5. send
                send_window[next_send % WINDOW_SIZE] = ( char* )malloc( segment_len );
                memcpy( send_window[next_send % WINDOW_SIZE], segment, segment_len );
                sendto( dest_socket, send_window[next_send % WINDOW_SIZE], segment_len, 0, ( struct sockaddr* ) &dest, dest_len );
                next_send++;
                nextseqnum += payload_len;

                printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0010 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
                seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );
            }
        }
        else
        {
            puts("overflow");
            sleep( 5 );
        }

        usleep( 500000 );
    }
    puts("EXIT");
    sleep( 3 );
    while( 1 )
    {
        pthread_mutex_lock( &lock_nextseqacknum );
        if( is_rcv )
        {
            puts("is receive\n");
            is_rcv = 0;
//            printf("last acked : %hu, next send : %d, next seq num : %u\n", last_acked, next_send, nextseqnum );
            for( temp_i = last_acked ; temp_i < next_send ; temp_i++ )
            {
                if( send_window[temp_i % WINDOW_SIZE] != NULL )
                {
                    if( *( uint32_t* )( rcv_tcp_header + 8 ) > *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) )
                    {
                        printf("free : %d, seqnum = %u\n", temp_i, *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) );
                        free( send_window[temp_i % WINDOW_SIZE] );
                        send_window[temp_i % WINDOW_SIZE] = NULL;
                        last_acked = temp_i + 1;
                    }
                    else if( *( uint32_t* )( rcv_tcp_header + 8 ) == *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ) )
                    {
//      5. send
//                        *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 8 ) = *( uint32_t* )( rcv_tcp_header + 4 );
                        sendto( dest_socket, send_window[temp_i % WINDOW_SIZE], PSEUDO_HEADER_LENGTH + *( uint16_t* )( send_window[ temp_i % WINDOW_SIZE] + 10), 0, ( struct sockaddr* ) &dest, dest_len );

                        printf("\n!!! Resend a packet(%s) to %s : %hu\t\n\n", identify_flags( 0x0010 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
                        seq_ack_num_info( *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 4 ), *( uint32_t* )( send_window[temp_i % WINDOW_SIZE] + PSEUDO_HEADER_LENGTH + 8 ), 1 );
                    }

                }
            }
        }
        if( last_acked == next_send )
        {
            is_rcv = -1;
            puts("\nLets FIN!!!\n");
            pthread_mutex_unlock( &lock_nextseqacknum );
            break;
        }
        pthread_mutex_unlock( &lock_nextseqacknum );
        sleep( 2 );
    }
    puts("PREPARE FIN");

    pthread_join( receive_ack, NULL );
//  send FIN
//      1. set payload
    memset( payload, 0, PAYLOAD_SIZE );
    payload_len = 0;

//      2. set tcp header
    source_port = ( uint16_t )atoi( src_port );
    destination_port = ( uint16_t )atoi( dest_port );
    seq_num = nextseqnum;
    ack_num = nextacknum;
    data_offset_flags = HEADER_LENGTH;
    data_offset_flags = ( data_offset_flags << 16 ) + 0x0001;
    win_size = WINDOW_SIZE;
    checksum = 0;
    ugn_ptr = 0;

    set_tcp_header( tcp_header, source_port, destination_port, seq_num, ack_num, data_offset_flags, win_size, checksum, ugn_ptr );

//      3.set pseudo header
    source_addr = src.sin_addr.s_addr;
    destination_addr = dest.sin_addr.s_addr;
    zeros_protocol = 6;
    payload_len = 0;
    tcp_len = HEADER_LENGTH + payload_len;

    set_pseudo_header( pseudo_header, source_addr, destination_addr, zeros_protocol, tcp_len );

//      4. build segment
    build_segment( segment, pseudo_header, tcp_header, payload, payload_len );
    segment_len = PSEUDO_HEADER_LENGTH + HEADER_LENGTH + payload_len;
    checksum = cumulate_checksum( segment, tcp_len + 12 );
    *( uint16_t* )( segment + PSEUDO_HEADER_LENGTH + 16 ) = checksum;

    sendto( dest_socket, segment, segment_len, 0, ( struct sockaddr* ) &dest, dest_len );
    printf("Send a packet(%s) to %s : %hu\t\n", identify_flags( 0x0001 ), inet_ntoa( dest.sin_addr ), ntohs( dest.sin_port ) );
    seq_ack_num_info( *( uint32_t* )( tcp_header + 4 ), *( uint32_t* )( tcp_header + 8 ), 1 );
//--------------------------------------------------------------------------------------------------------------------

    fclose( file );
//--------------------------------------------------------------------------------------------------------------------

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
