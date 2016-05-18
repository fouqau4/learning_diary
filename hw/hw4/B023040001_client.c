#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <endian.h>
#define BUF_SIZE 1500
#define PAYLOAD_SIZE 1024
#define HEADER_LENGTH 20
#define PSEUDO_HEADER_LENGTH 12

char buf[BUF_SIZE];

uint16_t cumulate_checksum( char* pseudo_header, char* tcp_header, char* payload, int payload_len )
{
    int temp_i;
    uint32_t temp_ui32t;
//  cumulate checksum ( pseudo header + tcp header + tcp payload )
    uint32_t checksum_temp = 0;
//      1. pseudo header
    for( temp_i = 0 ; temp_i < PSEUDO_HEADER_LENGTH ; temp_i += 2 )
    {
        checksum_temp += ( uint32_t )( *( uint16_t* )( pseudo_header + temp_i ) );
        while( checksum_temp > 0xffff )
        {
            temp_ui32t = checksum_temp / 0xffff;
            checksum_temp = checksum_temp % 0x10000 + temp_ui32t;
        }
    }
//      2. tcp header
    for( temp_i = 0 ; temp_i < HEADER_LENGTH ; temp_i += 2 )
    {
        checksum_temp += ( uint32_t )( *( uint16_t* )( tcp_header + temp_i ) );
        while( checksum_temp > 0xffff )
        {
            temp_ui32t = checksum_temp / 0xffff;
            checksum_temp = checksum_temp % 0x10000 + temp_ui32t;
        }
    }
//      3. tcp payload
    for( temp_i = 0 ; temp_i < payload_len ; temp_i += 2 )
    {
        checksum_temp += ( uint32_t )( *( uint16_t* )( payload + temp_i ) );
        while( checksum_temp > 0xffff )
        {
            temp_ui32t = checksum_temp / 0xffff;
            checksum_temp = checksum_temp % 0x10000 + temp_ui32t;
        }
    }
    return ( uint16_t )checksum_temp;
}

void run_cli( char* dest_ip, char* dest_port, char* src_port )
{
    int temp_i;
    uint32_t temp_ui32t;
    int payload_len = 0;
    char payload[PAYLOAD_SIZE];
    char segment[PSEUDO_HEADER_LENGTH + HEADER_LENGTH + PAYLOAD_SIZE];
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
    *( uint16_t* )tcp_header = htons( source_port );
//      set Destination port
    uint16_t destination_port = ( uint16_t )atoi( dest_port );
    *( uint16_t* )( tcp_header + 2 ) = htons( destination_port );
//      set Sequence number
    uint32_t seq_num = 0;
    *( uint32_t* )( tcp_header + 4 ) = htonl( seq_num );
//      set Ack number
    uint32_t ack_num = 0;
    *( uint32_t* )( tcp_header + 8 ) = htons( ack_num );
//      set Data offset
    uint16_t header_len = HEADER_LENGTH;
    header_len = header_len << 16;
//      set Flags
    uint16_t flags = 0;
//      set Window size
    uint16_t win_size = 0;
    *( uint16_t* )( tcp_header + 14 ) = htons( win_size );
//      set Checksum
    uint16_t checksum = 0;
//      set Urgent pointer
    uint16_t ugn_ptr = 0;
    *( uint16_t* )( tcp_header + 18 ) = htons( ugn_ptr );
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
    *( uint8_t* )( pseudo_header + 8 ) = htons( zeros_protocol );
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
    *( uint16_t* )( tcp_header + 12 ) = htons( header_len + flags );
    memset( payload, 0, PAYLOAD_SIZE );
    checksum = cumulate_checksum( pseudo_header, tcp_header, payload, payload_len );
    printf("checksum = %x\n", checksum );

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

//    uint16_t a = 0xffff;uint32_t b = a + 0xffff;
//    if( b % 0x10000 == 0xffff - 0x1 )
//    printf("%d", b % 0x10000 );

    return 0;
}
