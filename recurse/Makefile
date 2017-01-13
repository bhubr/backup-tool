CFLAGS=-g -v
# -da -Q

all: build

build: listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o post.o
	gcc -o listdir listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o post.o -ljansson

listdir.o: listdir.c ../http/post.h run_md5.h
	gcc -c listdir.c -Wno-pointer-sign $(CFLAGS)

run_md5.o: run_md5.c run_md5.h
	gcc -c run_md5.c $(CFLAGS)

post.o: ../http/post.c ../http/post.h
	gcc -o ./post.o -c ../http/post.c $(CFLAGS)

clean:
	rm *.o listdir