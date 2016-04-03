#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#define MAX_FILE_SIZE 1048575

char* decimalToBinary( int dec, int length )
{
    char *binary;
    binary = (char*)malloc( length * sizeof(char) + 1 ); memset( binary, 0, sizeof(binary) );
    printf("dec = %d ,length = %d , strlen = %ld, sizeof = %ld\n",dec, length, strlen(binary), sizeof(binary));
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
    puts(binary);
    return binary;
}

int binaryToDecimal( char* binary)
{
    int length = strlen(binary);
    puts(binary);
    printf("strlen of binary = %d\n", length);
    int decimal = 0;
    int temp_i;
    for( temp_i = 0 ; temp_i < length ; temp_i++ )
    {
        decimal += (int)(pow( (double)2, (double)(length - temp_i -1) ) * (binary[temp_i] - '0'));
    }
    return decimal;
}

void fixLengthTable(char* fileName)
{
    FILE *target;
    if( (target = fopen( fileName, "r")) == 0 )
        perror("fixLengthTable() : fopen() : target!!\n"), exit(1);
    //finding which character arised,and building an encoding table
    int check[256];
    memset( check, 0, sizeof(check) );
    unsigned char encodingTable[256];
    memset( encodingTable, 0, sizeof(encodingTable) );
    int kinds = 0;
    unsigned char temp;
    temp = (unsigned char)fgetc(target);
    while( kinds < 256 && !feof(target))
    {
        if( check[temp] == 0 )
            check[temp] = 1, encodingTable[(int)temp] = (unsigned char)kinds, kinds++;
        temp = (unsigned char)fgetc(target);
    }

    fclose(target);

    //exporting table
    char tableName[sizeof(fileName)];
    strcpy( tableName, fileName);
    strcat( tableName, "_table");
    FILE *table;
    if( (table = fopen( tableName, "w")) == 0 )
        perror("fixLengthTable() : fopen() : table!!\n"), exit(1);
    int i;

    fputc( (unsigned char)kinds, table);
    for( i=0; i < 256; i++ )
    {
        if(check[i] == 1)
        {
            fprintf( table, "%c%c", (unsigned char)i, encodingTable[i] );
            printf("%d_%d\n", (unsigned char)i, encodingTable[i] );
        }
    }
    fflush(table);
    fclose(table);

    puts("success building table");
}


void fCompression(char* fileName)
{
    //fetching encoding table
    FILE *table;
    char tableName[sizeof(fileName) + 6];
    strcpy( tableName, fileName );
    strcat( tableName, "_table");

    if( (table = fopen( tableName, "r")) == 0)
        perror("fCompression() : fopen() : table!!\n"), exit(1);

    unsigned char* encodingTable[256];
    memset( encodingTable, 0, sizeof(encodingTable) );
    unsigned char num;
    unsigned char tempA_c2[2];
    int codeLength = 0;
    int temp_i;
    fread( &num, sizeof(num), 1, table);
    temp_i = (int)num;
    while( temp_i > 0 )
        temp_i /= 2, codeLength++;
    for( temp_i=0; temp_i < num; temp_i++ )
    {
        fread( tempA_c2, sizeof(tempA_c2), 1, table );
        encodingTable[(int)tempA_c2[0]] = decimalToBinary( (int)tempA_c2[1], codeLength );
        puts(encodingTable[(int)tempA_c2[0]]);
    }
    fclose(table);

    //reading whole target file
    FILE *target;
    if( (target = fopen( fileName, "r" )) == 0 )
        perror("fCompression() : fopen() : target!!\n"), exit(1);

    long targetSize;
    fseek( target, 0, SEEK_END );
    targetSize = ftell(target);
    rewind(target);

    unsigned char readBuf[MAX_FILE_SIZE];
    memset( readBuf, 0, sizeof(readBuf) );
    fread( readBuf, targetSize, 1, target );
    fclose(target);

    //encoding target file into binary code
    unsigned char *binaryCodeBuf = (unsigned char*)malloc( codeLength * targetSize + 1 ); memset( binaryCodeBuf, 0 , sizeof(binaryCodeBuf));
    for( temp_i = 0 ; temp_i < targetSize ; temp_i++ )
    {
        printf("%c = %s \n", readBuf[temp_i], encodingTable[(int)readBuf[temp_i]]);
        printf(" %s + ", binaryCodeBuf );
        strcat( binaryCodeBuf, encodingTable[(int)readBuf[temp_i]] );
        printf("%s = %s \n", encodingTable[(int)readBuf[temp_i]], binaryCodeBuf);
    }
    //zero padding
    long int binaryCodeLength = strlen(binaryCodeBuf);
    unsigned char paddingNum;
    if(binaryCodeLength % 8 != 0)
    {
        paddingNum = (unsigned char)( 8 - binaryCodeLength % 8 );
        for( temp_i = 0 ; temp_i < (int)paddingNum ; temp_i++ )
            strcat( binaryCodeBuf, "0" );
    }
    FILE *result;
    char resultName[sizeof(fileName) + 7 ];
    strcpy( resultName, fileName );
    strcat( resultName, ".result");
    if( (result = fopen( resultName, "w" )) == 0 )
        perror("fCompression() : fopen() : result!!\n"), exit(1);

    fwrite( &paddingNum, sizeof(paddingNum), 1, result );

    puts(binaryCodeBuf);
    binaryCodeLength = strlen(binaryCodeBuf);
    printf("sizeof binaryCodeBuf = %ld\n", strlen(binaryCodeBuf));

    //writing encoded contents to result file
    unsigned char writeBuf[MAX_FILE_SIZE];
    memset( writeBuf, 0, sizeof(writeBuf) );
    int temp_j;
    unsigned char temp_c8[8];
    for( temp_i=0; temp_i < binaryCodeLength / 8 ; temp_i++ )
    {
        strncpy( temp_c8, binaryCodeBuf+(temp_i * 8), 8 );
        puts(temp_c8);
        writeBuf[temp_i] = (unsigned char)binaryToDecimal(temp_c8);
        printf("%d\n", (int)writeBuf[temp_i]);
    }
    free(binaryCodeBuf);

    //export compressed file
    fwrite( writeBuf, strlen(writeBuf) , 1, result );
    fclose(result);
    for( temp_i= 0; temp_i < 256 ; temp_i++ )
        free(encodingTable[temp_i]);

}

void fUncomperssion(char* fileName)
{
    //fetching encoding table
    FILE *table;
    char tableName[sizeof(fileName)-1];
    strncpy( tableName, fileName, sizeof(fileName) - 7 );
    strcat( tableName, "_table" );
    if( (table = fopen( tableName, "r")) == 0)
        perror("fUncompression : fopen : tableName!!\n"), exit(1);

    unsigned char* encodingTable[256];
    unsigned char recoverTable[256];
    memset( recoverTable, 0, sizeof(recoverTable) );
    memset( encodingTable, 0, sizeof(encodingTable) );
    unsigned char num;
    unsigned char tempA_c2[2];
    int codeLength = 0;
    int temp_i;
    fread( &num, sizeof(num), 1, table);
    temp_i = (int)num;
    while( temp_i > 0 )
        temp_i /= 2, codeLength++;
    for( temp_i=0; temp_i < num; temp_i++ )
    {
        fread( tempA_c2, sizeof(tempA_c2), 1, table );
        recoverTable[(int)tempA_c2[1]] = tempA_c2[0];
        encodingTable[(int)tempA_c2[0]] = decimalToBinary( (int)tempA_c2[1], codeLength );
//        puts(encodingTable[(int)tempA_c2[0]]);
    }
    fclose(table);

    //reading compression file
    FILE *target;
    if( (target = fopen( fileName, "r")) == 0 )
        perror("fUncompression : fopen : target!!\n"), exit(1);

    unsigned char paddingNum;
    unsigned char readBuf[MAX_FILE_SIZE];
    memset( readBuf, 0, sizeof(readBuf) );
    fseek( target, 0, SEEK_END);
    long targetSize = ftell(target) - 1;
    rewind(target);

    fread( &paddingNum, sizeof(paddingNum), 1, target );
    fread( readBuf, targetSize, 1, target );
    fclose(target);

    //converting code into binary bode
    unsigned char *binary = (unsigned char*)malloc( 8 * targetSize ); memset( binary, 0, sizeof(binary) );

    for( temp_i = 0; temp_i < targetSize ; temp_i++ )
    {
//        printf("binary = %s , next byte = %d \n", binary, (int)readBuf[temp_i]);
        strcat( binary, decimalToBinary( (int)readBuf[temp_i], 8 ) );
//        printf("new binary = %s \n", binary);
    }
    printf("binary code = %s \n",binary);
    int binaryCodeLength = strlen(binary);
    //uncompressing target file
    unsigned char writeBuf[MAX_FILE_SIZE];
    memset( writeBuf, 0, sizeof(writeBuf) );
    unsigned char tempA_c3[3];
    for( temp_i=0; temp_i < (binaryCodeLength - paddingNum) / codeLength ; temp_i++ )
    {
        strncpy( tempA_c3, binary + temp_i * codeLength, codeLength );
        puts(tempA_c3);
        writeBuf[temp_i] = recoverTable[binaryToDecimal(tempA_c3)];
        printf("%d\n", writeBuf[temp_i]);
    }
    puts(writeBuf);

    FILE *result;
    int resultNameLength = sizeof(fileName) - 6;
    char resultName[resultNameLength];
    printf("%ld\n", sizeof(fileName) -7);
    strncpy( resultName, fileName, sizeof(fileName) - 7 );
    resultName[resultNameLength] = '\0';
    puts(resultName);
    if( (result = fopen( resultName, "w" )) == 0 )
        perror("fUncompress() : fopen : result!!\n"), exit(1);

    fwrite( writeBuf, sizeof(targetSize), 1, result );
    fclose(result);


}
int main(int argc, char*argv[])
{
    if(argc==2)
    {
        fixLengthTable(argv[1]);
        fCompression(argv[1]);
    }
    else
    {
        fUncomperssion(argv[1]);
    }
    puts("LLL");
    return 0;
}
