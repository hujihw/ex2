CC=g++
CXXFLAGS=--std=c++11 -I. -Wall

.PHONY: tar sanity clean

libuthreads.a: uthreads.o
	ar rcs libuthreads.a uthreads.o

uthreads.o: uthreads.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $^

tar: uthreads.cpp Makefile README
	tar cvf ex2.tar $^

sanity: uthreads.h uthreads.cpp Makefile README
	tar cvf ex2.tar $^

clean:
	rm uthreads.o ex2.tar libuthreads.a
