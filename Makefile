CC=g++
CXXFLAGS=--std=c++11 -I. -Wall

.PHONY: all tar clean

all: libuthreads.a

uthreads.o: uthreads.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $^

libuthreads.a: uthreads.o
	ar rcs libuthreads.a uthreads.o

tar: uthreads.cpp Makefile README
	tar cvf ex2.tar $^

clean:
	rm uthreads.o ex2.tar libuthreads.a
