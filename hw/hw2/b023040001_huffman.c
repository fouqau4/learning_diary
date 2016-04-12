#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"b023040001_huffman.h"
//#define DEBUG



void decimalToBinary( int dec, unsigned char* binary, int length )
{
    memset( binary, 0, length + 1 );

#ifdef DEBUG
    printf("[DBG] %s : dec = %d ,length = %d\n", __FUNCTION__, dec, length );
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
}

int binaryToDecimal( unsigned char* binary)
{
    int length = strlen(binary);

#ifdef DEBUG
    printf("[DBG] %s : binary = %s, strlen of binary = %d\n", __FUNCTION__, binary, length );
#endif // DEBUG

    int decimal = 0;
    int temp_i;
    for( temp_i = 0 ; temp_i < length ; temp_i++ )
    {
        decimal += (int)(pow( (double)2, (double)(length - temp_i -1) ) * (binary[temp_i] - '0'));
    }

#ifdef DEBUG
    printf("[DBG] %s : decimal = %d\n\n", __FUNCTION__, decimal );
#endif // DEBUG

    return decimal;
}

int fixLengthTable(char* fileName)
{
    puts("\\(^_ ^)\\ starting to create encoding table");
    FILE *target; //the file which will be compressed
    if( (target = fopen( fileName, "r")) == 0 )
    {
        perror("fixLengthTable() : fopen() : target!!\n");
        return 1;
    }
    //-------------------------------------------------------------------------------------------------
    //counting # of code
    int encodingTable[256];
    int temp_i;
    for( temp_i = 0 ; temp_i < 256 ; temp_i++ )
        encodingTable[temp_i] = -1;
    int kinds = 0; //kinds of code
    unsigned char temp_uc; //temp of unsigned char


    //finding which character arised, and building an encoding table
    temp_uc = (unsigned char)fgetc(target);
    while( !feof(target) )
    {

#ifdef DEBUG
        printf("\n[DBG] %s() : line_%d : next char = %d, kinds = %d\n", __FUNCTION__, __LINE__, (int)temp_uc, kinds );
#endif // DEBUG

        if( encodingTable[temp_uc] == -1 )
        {
            encodingTable[(int)temp_uc] = kinds;
            kinds++;
        }
        temp_uc = (unsigned char)fgetc(target);
    }
    fclose(target);

    //exporting table
    int temp_nameLength = strlen(fileName) + 1 + 6 ;
    char tableName[temp_nameLength];
    memset( tableName, 0, sizeof(tableName) );
    strcpy( tableName, fileName);
    strcat( tableName, "_table");
    tableName[temp_nameLength] = '\0';

    FILE *table;
    if( (table = fopen( tableName, "w")) == 0 )
    {
        perror("fixLengthTable() : fopen() : table!!\n");
        return 1;
    }

    char integer[4];
    *(int*)integer = kinds;
    fwrite( integer, 4, 1, table );
    for( temp_i = 0 ; temp_i < 256 ; temp_i++ )
    {
        if( encodingTable[temp_i] != -1 )
        {
            fprintf( table, "%c%c", temp_i, (unsigned char)encodingTable[temp_i] );
#ifdef DEBUG
            printf("[DBG] %s : encode %3d into : %3d\n", __FUNCTION__, temp_i, encodingTable[temp_i] );
#endif // DEBUG
        }
    }
    fclose(table);
    fflush(table);
    //-------------------------------------------------------------------------------------------------

    puts("\\(^_ ^)\\ succeed in building encoding table\n");
}


int fCompression(char* fileName)
{
    puts("\\(^_ ^)\\ starting to compress file\n");
    puts("\\(^_ ^)\\ creating encoding table");
    if( fixLengthTable(fileName) == 1 )
    {
        perror("fCompression() : fixLengthTable() : fail to build table!!\n");
        return 1;
    }
    //-------------------------------------------------------------------------------------------------
    //fetching encoding table
    puts("\\(^_ ^)\\ fetching encoding table");
    FILE *table; //encoding table
    int temp_nameLength = strlen(fileName) + 1 + 6;
    char tableName[temp_nameLength];
    memset( tableName, 0, sizeof(tableName) );
    strcpy( tableName, fileName );
    strcat( tableName, "_table");
    tableName[temp_nameLength] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : tableName = %s\n", __FUNCTION__, tableName);
#endif // DEBUG

    if( (table = fopen( tableName, "r")) == 0)
    {
        perror("fCompression() : fopen() : table!!\n");
        return 1;
    }

//    unsigned char* encodingTable[256];  memset( encodingTable, 0, sizeof(encodingTable) ); //encodingTable keeping binary code
    unsigned char encodingTable[256];
    memset( encodingTable, 0, sizeof(encodingTable) ); //encodingTable keeping binary code
    int kinds;
    unsigned char tempA_uc_2[2]; //temp array of unsigned char with size 2
    int codeLength = 0; //length of binary code
    int temp_i; //temp of integer

    char integer[4];
    fread( integer, 4, 1, table );
    kinds = *(int*)integer;
    //calaulating codeLength
    if( kinds > 1 )
        temp_i = kinds -1;
    while( temp_i > 0 )
        temp_i /= 2, codeLength++;

#ifdef DEBUG
    printf("[DBG] %s : kinds = %d , codeLength = %d\n", __FUNCTION__, kinds, codeLength);
#endif // DEBUG

    //filling encodingTable
    for( temp_i=0; temp_i < kinds ; temp_i++ )
    {
        fread( tempA_uc_2, 2, 1, table );
        encodingTable[(int)tempA_uc_2[0]] = tempA_uc_2[1];

#ifdef DEBUG
        printf("[DBG] %s : encodingTable[%d] = %d\n", __FUNCTION__, (int)tempA_uc_2[0], (int)tempA_uc_2[1] );
#endif // DEBUG

    }

    fclose(table);
    //-------------------------------------------------------------------------------------------------
    //reading whole target file
    puts("\\(^_ ^)\\  reading whole target file\n");

    FILE *target; //the file which will be compressed
    if( (target = fopen( fileName, "r" )) == 0 )
    {
        perror("fCompression() : fopen() : target!!\n");
        return 1;
    }

    long targetSize; //size of target
    fseek( target, 0, SEEK_END );
    targetSize = ftell(target);
    rewind(target);

#ifdef DEBUG
    printf("[DBG] %s : targetSize = %ld\n", __FUNCTION__, targetSize );
#endif // DEBUG
    //storing target in readBuf
    unsigned char *readBuf = (unsigned char*)malloc(targetSize);
    memset( readBuf, 0, targetSize );
    fread( readBuf, targetSize, 1, target );
    fclose(target);

    //-------------------------------------------------------------------------------------------------
    //zero padding
    puts("\\(^_ ^)\\ zero padding\n");

    long int binaryCodeLength = codeLength * targetSize; //length of target after encoding into binary code
    unsigned char paddingNum = 0;
    //calculating padding number
    if(binaryCodeLength % 8 != 0)
    {
        paddingNum = (unsigned char)( 8 - binaryCodeLength % 8 );
        binaryCodeLength += (int)paddingNum;
    }

#ifdef DEBUG
    printf("[DBG] %s : paddingNum = %d\n\n", __FUNCTION__, (int)paddingNum );
#endif // DEBUG
    //-------------------------------------------------------------------------------------------------
    //encoding target file into binary code
    puts("\\(^_ ^)\\ encoding target file into binary code\n");

    unsigned char *binaryCodeBuf = (unsigned char*)malloc( binaryCodeLength + 1 );
    memset( binaryCodeBuf, 0 , binaryCodeLength + 1);
    puts("\n**********************************************************");
    puts("*                                                        *");
    unsigned char tempS_uc_codeLength[codeLength + 1];
    for( temp_i = 0 ; temp_i < targetSize ; temp_i++ )
    {
        decimalToBinary( (int)encodingTable[(int)readBuf[temp_i]], tempS_uc_codeLength, codeLength );
#ifdef DEBUG
        printf("[DBG] %s : next charecter in readBuf : %d = %s \n", __FUNCTION__, (int)readBuf[temp_i], tempS_uc_codeLength );
#endif // DEBUG

        strcpy( binaryCodeBuf + temp_i * codeLength, tempS_uc_codeLength );
        printf("*   Compression Progress :  [%10d / %10ld ]   *\n", temp_i+1, targetSize); //showing progress
    }
    puts("*                                                        *");
    puts("**********************************************************\n\n");
    //filling binaryCode into multiple of byte
    for( temp_i = 0 ; temp_i < (int)paddingNum ; temp_i++ )
        strcat( binaryCodeBuf, "0" );

    free(readBuf);

    //-------------------------------------------------------------------------------------------------
    //writing encoded contents to result file
    puts("\\(^_ ^)\\ writing encoded contents to result file");

    FILE *result; //the file after compressing
    temp_nameLength = strlen(fileName) + 1 + 7;
    char resultName[temp_nameLength];
    memset( resultName, 0, sizeof(resultName) );
    strcpy( resultName, fileName );
    strcat( resultName, ".result");
    resultName[temp_nameLength] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : resultName = %s\n", __FUNCTION__, resultName );
#endif // DEBUG

    if( (result = fopen( resultName, "w" )) == 0 )
    {
        perror("fCompression() : fopen() : result!!\n");
        return 1;
    }
    //recording paddingNum in result
    fwrite( &paddingNum, sizeof(paddingNum), 1, result );

    binaryCodeLength = strlen(binaryCodeBuf); //updating binaryCodeLength after zero padding

#ifdef DEBUG
    printf("[DBG] %s : binaryCodeLength = %ld\n\n", __FUNCTION__, binaryCodeLength );
#endif // DEBUG

    unsigned char* writeBuf = (unsigned char*)malloc( binaryCodeLength / 8 + 1 );
    memset( writeBuf, 0, binaryCodeLength / 8 + 1 );
    unsigned char tempS_uc_8[8 + 1]; //temp array of unsigned char with size 8
    //converting 8 bytes binary code into 1 byte char, and writing into result
    for( temp_i=0; temp_i < binaryCodeLength / 8 ; temp_i++ )
    {
        strncpy( tempS_uc_8, binaryCodeBuf+(temp_i * 8), 8 );
        tempS_uc_8[8] = '\0';
#ifdef DEBUG
        printf("[DBG] %s : next byte : %s = %d, strlen = %zu, position = %d\n", __FUNCTION__, tempS_uc_8, binaryToDecimal(tempS_uc_8), strlen(tempS_uc_8), temp_i * 8 );
#endif // DEBUG

        writeBuf[temp_i] = (unsigned char)binaryToDecimal(tempS_uc_8);
    }

    fwrite( writeBuf, temp_i , 1, result );
    fclose(result);

//    for( temp_i= 0; temp_i < 256 ; temp_i++ )
//        if( encodingTable[temp_i] != NULL )
//            free(encodingTable[temp_i]);

    free(binaryCodeBuf);
    free(writeBuf);
    //-------------------------------------------------------------------------------------------------
    puts("\\(^_ ^)\\ succeed in compressing file\n");

    return 0;
}

int fUncompression(char* fileName)
{
    //-------------------------------------------------------------------------------------------------
    //fetching encoding table
    puts("/( - v-)/ fetching encoding table\n");

    FILE *table; //encoding table
    int temp_nameLength = strlen(fileName) + 1 - 7 + 6;
    char tableName[temp_nameLength];
    memset( tableName, 0, sizeof(tableName) );
    strncpy( tableName, fileName, strlen(fileName) - 7 );
    strcat( tableName, "_table" );
    tableName[temp_nameLength] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : tableName = %s\n", __FUNCTION__, tableName );
#endif // DEBUG

    if( (table = fopen( tableName, "r")) == 0)
    {
        perror("fUncompression : fopen : tableName!!\n");
        return 1;
    }
    int temp_i; //temp of integer

    unsigned char recoverTable[256];
    memset( recoverTable, 0, sizeof(recoverTable) );

    int kinds; //kinds of code
    unsigned char tempA_uc_2[2];
    int codeLength = 0;

    char integer[4];
    fread( integer, 4, 1, table );
    kinds = *(int*)integer;
    //calculating codeLength
    if( kinds > 1 )
        temp_i = kinds - 1;
    while( temp_i > 0 )
        temp_i /= 2, codeLength++;

#ifdef DEBUG
    printf("[DBG] %s : kinds = %d, codeLength = %d\n", __FUNCTION__, kinds, codeLength );
#endif // DEBUG

    for( temp_i=0; temp_i < kinds ; temp_i++ )
    {
        fread( tempA_uc_2, 2, 1, table );
        recoverTable[(int)tempA_uc_2[1]] = tempA_uc_2[0];

        #ifdef DEBUG
        printf("\n[DBG] %s() : line_%d : recoverTable[%d] = %d\n", __FUNCTION__, __LINE__, (int)tempA_uc_2[1], (int)tempA_uc_2[0] );
        #endif // DEBUG

    }
    fclose(table);
    //-------------------------------------------------------------------------------------------------
    //reading compressed file
    puts("/( - v-)/ reading compressed file\n");

    FILE *target; //the file which will be uncompressed
    if( (target = fopen( fileName, "r")) == 0 )
    {
        perror("fUncompression : fopen : target!!\n");
        return 1;
    }

    //fileSize of target
    fseek( target, 0, SEEK_END);
    long targetSize = ftell(target) - 1;
    rewind(target);

#ifdef DEBUG
    printf("[DBG] %s : targetSize = %ld\n", __FUNCTION__, targetSize );
#endif // DEBUG

    unsigned char paddingNum;
    fread( &paddingNum, sizeof(paddingNum), 1, target );

#ifdef DEBUG
    printf("[DBG] %s : paddingNum = %d\n", __FUNCTION__, (int)paddingNum );
#endif // DEBUG

    unsigned char *readBuf = (unsigned char*)malloc( targetSize );
    memset( readBuf, 0, targetSize );
    fread( readBuf, targetSize, 1, target );

    fclose(target);

    //-------------------------------------------------------------------------------------------------
    //converting code into binary code
    puts("/( - v-)/ converting code into binary code\n");

    int binaryCodeLength = 8 * targetSize + 1;

#ifdef DEBUG
    printf("[DBG] %s : binaryCodeLength = %d\n", __FUNCTION__, binaryCodeLength );
#endif // DEBUG

    unsigned char *binary = (unsigned char*)malloc( binaryCodeLength );
    memset( binary, 0, binaryCodeLength );
    unsigned char tempS_uc_8[8 + 1];
    for( temp_i = 0; temp_i < targetSize ; temp_i++ )
    {
        decimalToBinary( (int)readBuf[temp_i], tempS_uc_8, 8 ) ;
#ifdef DEBUG
        printf("[DBG] %s : next charecter in readBuf : %d = %s\n", __FUNCTION__, (int)readBuf[temp_i], tempS_uc_8 );
#endif // DEBUG

        strcat( binary, tempS_uc_8 );
    }
    binary[binaryCodeLength] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : binary (code) = %s\n", __FUNCTION__, binary );
#endif // DEBUG
    //-------------------------------------------------------------------------------------------------
    //uncompressing target file
    puts("/( - v-)/ uncompressing target file\n");
    int temp_i1 = ( binaryCodeLength - 1 - paddingNum ) / codeLength;
    unsigned char *writeBuf = (unsigned char*)malloc( temp_i1 );
    memset( writeBuf, 0, temp_i1 );

    unsigned char tempS_uc_codeLength[codeLength + 1];
    int resultLength = 0;
    for( temp_i=0; temp_i < temp_i1 ; temp_i++ )
    {
        strncpy( tempS_uc_codeLength, binary + temp_i * codeLength, codeLength );
        tempS_uc_codeLength[codeLength] = '\0';
#ifdef DEBUG
        printf("[DBG] %s : current tempS_uc_codeLength = %s = %d ---> %d\n", __FUNCTION__, tempS_uc_codeLength, binaryToDecimal( tempS_uc_codeLength ), recoverTable[binaryToDecimal( tempS_uc_codeLength )] );
#endif // DEBUG

        writeBuf[temp_i] = recoverTable[binaryToDecimal( tempS_uc_codeLength )];
    }

    free(binary);
    //-------------------------------------------------------------------------------------------------
    //export uncompressed file
    puts("/( - v-)/ export uncompressed file\n");

    FILE *result;
    temp_nameLength = (int)strlen(fileName) +1 - 7;
    char resultName[temp_nameLength];
    memset( resultName, 0, sizeof(resultName) );
    strncpy( resultName, fileName, strlen(fileName) - 7 );
    resultName[temp_nameLength] = '\0';

#ifdef DEBUG
    printf("[DBG] %s : resultName = %s\n", __FUNCTION__, resultName );
#endif // DEBUG

    if( (result = fopen( resultName, "w" )) == 0 )
    {
        perror("fUncompress() : fopen : result!!\n");
        return 1;
    }
    fwrite( writeBuf, temp_i, 1, result );
    fclose(result);
    //-------------------------------------------------------------------------------------------------
    free(writeBuf);
    free(readBuf);

    return 0;
}
