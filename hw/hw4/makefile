all : server client

server : srv.o mylib.o
	gcc -o server B023040001_server.c cubelib.c

client : cli.o mylib.o
	gcc -o client B023040001_client.c cubelib.c

srv.o : B023040001_server.c mylib.o
	gcc -c B023040001_server.c

cli.o : B023040001_client.c mylib.o
	gcc -c B023040001_client.c

mylib.o : cubelib.c
	gcc -c cubelib.c
clear : 
	rm -f *.o server
