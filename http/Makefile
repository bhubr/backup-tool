all: build

build: post.o test_post.o
	gcc -o test_post post.o test_post.o

post.o: post.c post.h
	gcc -c post.c

test_post.o: test_post.c
	gcc -c test_post.c
