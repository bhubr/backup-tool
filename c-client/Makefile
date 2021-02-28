CFLAGS=-I. -Wall
LDFLAGS=-lcurl -ljansson

clean :
	rm client client.o file_utils.o requests.o

client : client.o file_utils.o requests.o
	gcc -o client client.o file_utils.o requests.o $(CFLAGS) $(LDFLAGS)

client.o : client.c
	gcc -c -o client.o client.c $(CFLAGS)

file_utils.o : file_utils.c
	gcc -c -o file_utils.o file_utils.c $(CFLAGS)

requests.o : requests.c
	gcc -c -o requests.o requests.c $(CFLAGS)