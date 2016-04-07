#include<stdio.h>
#include"b023040001_huffman.h"
int main(int argc, char* argv[])
{
    if( argc == 2)
        fCompression(argv[1]);
    else
        fUncomperssion(argv[1]);

    return 0;
}
