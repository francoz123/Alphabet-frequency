COMPILER = gcc
CFLAGS = -Wall -pedantic
EXES = assignment1.o histogram.o ring_of_process.o file_operations.o assignment1.o assignment1

assignment1: assignment1.o histogram.o ring_of_process.o file_operations.o
	${COMPILER} ${CFLAGS} assignment1.o histogram.o ring_of_process.o file_operations.o -o assignment1

assignment1.o: histogram.h ring_of_process.h file_operations.h
	 ${COMPILER} ${CFLAGS} -c assignment1.c -o assignment1.o

histogram.o: histogram.c histogram.h
	${COMPILER} ${CFLAGS} -c histogram.c -o histogram.o

ring_of_process.o: ring_of_process.c ring_of_process.h
	${COMPILER} ${CFLAGS} -c ring_of_process.c  -o ring_of_process.o

file_operations.o: file_operations.c file_operations.h
	${COMPILER} ${CFLAGS} -c file_operations.c -o file_operations.o
clean:
	rm -f *~ *.o ${EXES}
run: 
	./assignment1 2 text_files
