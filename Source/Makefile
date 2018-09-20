CC=gcc -Wall -g
C=clang -g -Wall -o
files=list.c thread_support.c mythreads.c
hfiles=list.h thread_support.h mythreads.h datatypes.h
lib=libmythreads.a

project2: $(files)
	$(CC) -c $(files)
	ar -cvr $(lib) *.o

clean:
	rm *.o
	rm -f $(lib)
