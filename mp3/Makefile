all: build

build: mp3_checksum.o test_mp3_checksum.o md5.o
	gcc -o test_mp3_checksum mp3_checksum.o test_mp3_checksum.o md5.o

md5.o: md5.c md5.h
	gcc -c md5.c

mp3_checksum.o: mp3_checksum.c mp3_checksum.h
	gcc -c mp3_checksum.c

test_mp3_checksum.o: test_mp3_checksum.c
	gcc -c test_mp3_checksum.c
