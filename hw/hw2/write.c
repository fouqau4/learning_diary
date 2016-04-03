#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main()
{
    FILE *write;
    if( (write = fopen("writeTest","w")) == 0)
        perror("fail fopen!!\n"),exit(1);
    char *buf = (char*)malloc(1000); memset( buf, 'A' , sizeof(buf));
//    gets(buf);
//    buf[3]='\0';
    printf("strlen = %u\n",strlen(buf));
    printf("sizeof = %lu\n",sizeof(buf));
    int temp_i;
    int a = 6;
    fputc((unsigned char)a,write);
    for( temp_i=0; temp_i<1000; temp_i+=2 ){
//        fputc(buf[temp_i],write);
        fprintf(write,"%c%c",buf[temp_i],buf[temp_i+1]);
    }
    //fprintf( write, "%s",buf);
    //fwrite( buf, sizeof(buf), 1, write);
    fclose(write);
    free(buf);
return 0;
}
