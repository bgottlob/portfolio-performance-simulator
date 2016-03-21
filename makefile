ser_portfolio.o : ser_portfolio.c
	gcc -c ser_portfolio.c -o ser_portfolio.o

par_portfolio.o : par_portfolio.c
	gcc -c -fopenmp par_portfolio.c -o par_portfolio.o

compile : par_portfolio.o ser_portfolio.o
	gcc ser_portfolio.o -lgsl -lgslcblas -lm -o ser_portfolio.out
	gcc par_portfolio.o -fopenmp -lgsl -lgslcblas -lm -o par_portfolio.out
	
clean :
	rm *.o *.out
