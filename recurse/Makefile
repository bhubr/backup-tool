all: build

build: listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o ../http/post.o
	gcc -o listdir listdir.o run_md5.o ../mp3/mp3_checksum.o ../mp3/md5.o ../http/post.o -ljansson

listdir.o: listdir.c
	gcc -c listdir.c

run_md5.o: run_md5.c run_md5.h
	gcc -c run_md5.c
