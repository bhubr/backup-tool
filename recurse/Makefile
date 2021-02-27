CFLAGS=
# -da -Q

all: build

build: listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o post.o utils.o
	gcc -o listdir listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o post.o utils.o -ljansson

listdir.o: listdir.c post.h run_md5.h
	gcc -c listdir.c -Wno-pointer-sign $(CFLAGS)

run_md5.o: run_md5.c run_md5.h
	gcc -c run_md5.c $(CFLAGS)

post.o: post.c post.h
	gcc -o ./post.o -c post.c $(CFLAGS)

utils.o: utils.c utils.h
	gcc -o ./utils.o -c utils.c $(CFLAGS)

clean:
	rm *.o listdir

ldir : ldir.c
	gcc -o ldir ldir.c