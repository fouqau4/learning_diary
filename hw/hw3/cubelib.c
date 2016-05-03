#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <assert.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0; // 我們已經送出多少 bytes 的資料
    int bytesleft = *len; // 我們還有多少資料要送
    int n;

    while(total < *len)
    {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
    }

    *len = total; // 傳回實際上送出的資料量

    return n==-1?-1:0; // 失敗時傳回 -1、成功時傳回 0
}


//--------------------------------------------------------------------------------------------------------------------
unsigned char* decimalToBinary( int dec, int length )
{
    unsigned char *binary;
    binary = (char*)malloc( length + 1 );
    memset( binary, 0, length + 1 );

#ifdef DEBUG
    printf("[DBG] %s : dec = %d ,length = %d , strlen = %ld, sizeof = %ld\n", __FUNCTION__, dec, length, strlen(binary), sizeof(binary));
#endif // DEBUG

    int temp_i;
    for( temp_i = length-1 ; temp_i >= 0; temp_i--)
    {
        if(dec > 0)
        {
            binary[temp_i] = dec%2 + '0';
            dec /= 2;
        }
        else
            binary[temp_i] = '0';
    }
    binary[length] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : binary = %s\n\n", __FUNCTION__, binary );
#endif // DEBUG

    return binary;
}

//--------------------------------------------------------------------------------------------------------------------
ssize_t write_all (int fd, const void* buffer, size_t count)
{

    size_t left_to_write = count;

    while (left_to_write > 0)
    {

        size_t written = write (fd, buffer, count);

        if (written == -1)

            /* An error occurred; bail.  */

            return -1;

        else

            /* Keep count of how much more we need to write.  */

            left_to_write -= written;

    }

    /* We should have written no more than COUNT bytes!   */

    assert (left_to_write == 0);

    /* The number of bytes written is exactly COUNT.  */

    return count;

}
//--------------------------------------------------------------------------------------------------------------------
