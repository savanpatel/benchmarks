all: build

default: build

build:
	gcc -o malloc-test malloc-test.c -pthread -lrt -g -Wall -Werror -lm
	./malloc-test

clean:
	rm -rf result.tsv malloc-test
