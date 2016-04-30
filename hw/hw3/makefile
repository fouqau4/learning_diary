all: server client

srv = b023040001_srv.c
cli = b023040001_cli.c

server: srv.o mylib.o
	gcc -o server $(srv) cubelib.c -lpthread -lm
client: cli.o mylib.o
	gcc -o client $(cli) cubelib.c -lpthread
srv.o: $(srv) mylib.o
	gcc -c $(srv) -lpthread
cli.o: $(cli) mylib.o
	gcc -c $(cli) -lpthread
mylib.o:
	gcc -c cubelib.c -lm

clear:
	rm -f *.o server client
