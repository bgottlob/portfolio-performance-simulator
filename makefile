port.o : portfolio.c
	gcc -Wall -g -I/usr/local/include -c portfolio.c -o port.o

compile : port.o
	gcc -L/usr/local/lib port.o -lgsl -lgslcblas -lm -o port.out

run : port.out
	./port.out

clean :
	rm *.o *.out
