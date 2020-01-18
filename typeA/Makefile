CC=mpicc
CFLAGS = -I/. -I./alpha -I./beta -I./gamma -I./delta -g

all: alpha/alpha-server.c alpha/alpha-client.c beta/beta-server.c beta/beta-client.c gamma/gamma-server.c gamma/gamma-client.c delta/delta-client.c delta/delta-server.c client.c server.c
	$(CC) -c $(CFLAGS) -fPIC alpha/alpha-server.c -o alpha-server.o
	$(CC) -c $(CFLAGS) -fPIC alpha/alpha-client.c -o alpha-client.o
	$(CC) -shared -o libalpha-client.so alpha-client.o
	$(CC) -shared -o libalpha-server.so alpha-server.o
	$(CC) -c $(CFLAGS) -fPIC beta/beta-server.c -o beta-server.o
	$(CC) -c $(CFLAGS) -fPIC beta/beta-client.c -o beta-client.o
	$(CC) -shared -o libbeta-client.so beta-client.o
	$(CC) -shared -o libbeta-server.so beta-server.o
	$(CC) -c $(CFLAGS) -fPIC gamma/gamma-server.c -o gamma-server.o
	$(CC) -c $(CFLAGS) -fPIC gamma/gamma-client.c -o gamma-client.o
	$(CC) -shared -o libgamma-client.so gamma-client.o
	$(CC) -shared -o libgamma-server.so gamma-server.o
	$(CC) -c $(CFLAGS) -fPIC delta/delta-server.c -o delta-server.o
	$(CC) -c $(CFLAGS) -fPIC delta/delta-client.c -o delta-client.o
	$(CC) -shared -o libdelta-client.so delta-client.o
	$(CC) -shared -o libdelta-server.so delta-server.o
	$(CC) -c $(CFLAGS) server.c -o server.o
	$(CC) -c $(CFLAGS) client.c -o client.o
	$(CC) -o server server.o -L. -lalpha-client -lalpha-server -lbeta-client -lbeta-server -lgamma-client -lgamma-server -ldelta-client -ldelta-server -lmargo $(LIBS) 
	$(CC) -o client client.o -L. -lalpha-client -lmargo

clean:
	rm *.o *.csv *.trace client server core* *.so server_addr* *.pdf hosts*
