#ifndef _CUBE_LIB_
#define _CUBE_LIB_

//--------------------------------------------------------------------------------------------------------------------

/*-------------------------------------------------------------------------------------------------------

        #ifdef DEBUG
        printf("\n[DBG] %s() : line_%d : \n", __FUNCTION__, __LINE__,  );
        #endif // DEBUG

    {
        fprintf( stderr, "\n[ERR] %s() : line_%d : ", __FUNCTION__, __LINE__ - 2 );
        perror("");
        exit( 1 );
    }
--------------------------------------------------------------------------------------------------------*/
int sendall(int s, char *buf, int *len);
unsigned char* decimalToBinary( int dec, int length );
ssize_t write_all (int fd, const void* buffer, size_t count);

#endif
