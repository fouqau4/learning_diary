#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_FILE_SIZE 1048575
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
    char tableName[sizeof(fileName)];
    strcpy( tableName, fileName );
    strcat( tableName, "_table");

    if( (table = fopen( tableName, "r")) == 0)
        perror("fCompression() : fopen() : table!!\n"), exit(1);

    unsigned char encodingTable[256];
    memset( encodingTable, 0, sizeof(encodingTable) );
    unsigned char num;
    unsigned char tempA_c[2];
    fread( &num, sizeof(num), 1, table);
    int temp_i;
    for( temp_i=0; temp_i < num; temp_i++ )
    {
        fread( tempA_c, sizeof(tempA_c), 1, table );
        encodingTable[(int)tempA_c[0]] = tempA_c[1];
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

    //writing encoded contents to result file
    unsigned char writeBuf[targetSize];
    memset( writeBuf, 0, sizeof(writeBuf) );
    for( temp_i=0; temp_i < targetSize; temp_i++ )
    {
        writeBuf[temp_i] = encodingTable[(int)readBuf[temp_i]];
    }
    FILE *result;
    char resultName[sizeof(fileName)];
    strcpy( resultName, fileName );
    strcat( resultName, ".result");
    if( (result = fopen( resultName, "w" )) == 0 )
        perror("fCompression() : fopen() : result!!\n"), exit(1);
    fwrite( writeBuf, sizeof(writeBuf), 1, result );
    fclose(result);

}

void fUncomperssion(char* fileName)
{
    //fetching encoding table
    FILE *table;
    char tableName[sizeof(fileName)-6];
    strncpy( tableName, fileName, sizeof(fileName)-6 );
    strcat( tableName, "_table" );
    if( (table = fopen( tableName, "r")) == 0)
        perror("fUncompression : fopen : tableName!!\n"), exit(1);

    unsigned char encodingTable[256];
    memset( encodingTable, 0, sizeof(encodingTable) );
    unsigned char num = fgetc(table);
    unsigned char tempA_c[2];
    int temp_i;
    for( temp_i=0; temp_i<(int)num ; temp_i++ )
    {
        fread( tempA_c, sizeof(tempA_c), 1, table );
        encodingTable[(int)tempA_c[1]] = tempA_c[0];
        printf("%d _ %d --%d\n", tempA_c[1], tempA_c[0], temp_i);
    }
    fclose(table);

    //reading compression file
    FILE *target;
    if( (target = fopen( fileName, "r")) == 0 )
        perror("fUncompression : fopen : target!!\n"), exit(1);

    unsigned char readBuf[MAX_FILE_SIZE];
    memset( readBuf, 0, sizeof(readBuf) );
    fseek( target, 0, SEEK_END);
    long targetSize = ftell(target);
    rewind(target);

    fread( readBuf, targetSize, 1, target );
    fclose(target);

    //uncompressing target file
    unsigned char writeBuf[MAX_FILE_SIZE];
    memset( writeBuf, 0, sizeof(writeBuf) );
    for( temp_i=0; temp_i<targetSize; temp_i++ )
    {
        writeBuf[temp_i] = encodingTable[readBuf[temp_i]];
    }
    puts(writeBuf);
    FILE *result;
    char resultName[sizeof(fileName)-6];
    strncpy( resultName, fileName, sizeof(fileName)-6 );
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
