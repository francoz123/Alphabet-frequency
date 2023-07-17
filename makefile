COMPILER = gcc
CFLAGS = -Wall -pedantic

a2: a2.c
	${COMPILER} ${CFLAGS} assignment1.c -o assignment1
clean:
	rm -f *~ *.o assignment1
run: 
	./assignment1 2 text_files
