CC=gcc -Wall -g
C=clang -g -Wall -o
files=list.c thread_support.c mythreads.c
hfiles=list.h thread_support.h mythreads.h datatypes.h
tfiles=threadcount.c threadpass.c threadcrash.c
lib=libmythreads.a

project2: $(files)
	$(CC) -c $(files)
	ar -cvr $(lib) *.o

test: $(lib) $(tfiles)
	$(C) threadcount threadcount.c $(lib)
	$(C) threadpass threadpass.c $(lib)
	$(C) threadcrash threadcrash.c $(lib)

clean:
	rm *.o
	rm -f $(lib)

tar:
	tar cvzf project2.tgz Makefile $(files) $(hfiles) README.txt

