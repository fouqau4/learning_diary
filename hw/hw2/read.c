#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
    FILE *i;
    if( (i = fopen(argv[1],"r"))== 0)
        perror("fopen fail!!\n"), exit(1);
    unsigned char c;
    int count = 1;
    c = fgetc(i);
    while(!feof(i))
    {
        printf("No.%d : %d \n", count++, (int)c );
        c=fgetc(i);
    }
    fclose(i);
    return 0;
}
