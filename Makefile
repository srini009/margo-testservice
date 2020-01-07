CC=gcc
CFLAGS = -I/. -g

all: alpha-server.c alpha-client.c client.c server.c
	gcc -c $(CFLAGS) -fPIC alpha-server.c -o alpha-server.o
	gcc -c $(CFLAGS) -fPIC alpha-client.c -o alpha-client.o
	gcc -c $(CFLAGS) server.c -o server.o
	gcc -c $(CFLAGS) client.c -o client.o
	gcc -shared -o libalpha-client.so alpha-client.o
	gcc -shared -o libalpha-server.so alpha-server.o
	gcc -o server server.o -L. -lalpha-server -lmargo
	gcc -o client client.o -L. -lalpha-client -lmargo

clean:
	rm *.o *.csv *.trace client server core* libalpha-client.so libalpha-server.so
