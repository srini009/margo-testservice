CC=mpicc
CFLAGS = -I/. -I/include -I./microservices/compute -I./microservices/memory -I./microservices/network -I./microservices/storage -g

all: ./microservices/compute/compute-server.c ./microservices/compute/compute-client.c ./microservices/memory/memory-server.c ./microservices/memory/memory-client.c ./microservices/network/network-server.c ./microservices/network/network-client.c ./microservices/storage/storage-client.c ./microservices/storage/storage-server.c  server.c client.c
	$(CC) -c $(CFLAGS) -fPIC ./microservices/compute/compute-server.c -o compute-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/compute/compute-client.c -o compute-client.o
	$(CC) -shared -o libcompute-client.so compute-client.o
	$(CC) -shared -o libcompute-server.so compute-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/memory/memory-server.c -o memory-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/memory/memory-client.c -o memory-client.o
	$(CC) -shared -o libmemory-client.so memory-client.o
	$(CC) -shared -o libmemory-server.so memory-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/network/network-server.c -o network-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/network/network-client.c -o network-client.o
	$(CC) -shared -o libnetwork-client.so network-client.o
	$(CC) -shared -o libnetwork-server.so network-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/storage/storage-server.c -o storage-server.o
	$(CC) -c $(CFLAGS) -fPIC ./microservices/storage/storage-client.c -o storage-client.o
	$(CC) -shared -o libstorage-client.so storage-client.o
	$(CC) -shared -o libstorage-server.so storage-server.o
	$(CC) -c $(CFLAGS) server.c -o server.o
	$(CC) -c $(CFLAGS) client.c -o client.o
	$(CC) -o server server.o -L. -lcompute-client -lcompute-server -lmemory-client -lmemory-server -lnetwork-client -lnetwork-server -lstorage-client -lstorage-server -lmargo $(LIBS) 
	$(CC) -o client client.o -L. -lnetwork-client -lmargo

clean:
	rm *.o *.csv *.trace *.stats client server core* *.so server_addr* *.pdf hosts* rm *_server.h *_client.h user_services.h *provider_ids*
