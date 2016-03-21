# Compile the shared library of portfolio functions
portfolio_lib.o : portfolio_lib.h portfolio_lib.c
	gcc -c portfolio_lib.c -o portfolio_lib.o

# Compile serial implementation
ser_portfolio.o : ser_portfolio.c
	gcc -c ser_portfolio.c -o ser_portfolio.o

# Compile parallel implementation
par_portfolio.o : par_portfolio.c
	gcc -c -fopenmp par_portfolio.c -o par_portfolio.o

# Links the libraries with both serial and parallel implementations to create executable files
compile : par_portfolio.o ser_portfolio.o portfolio_lib.o
	gcc portfolio_lib.o ser_portfolio.o -lgsl -lgslcblas -lm -o ser_portfolio.out
	gcc portfolio_lib.o par_portfolio.o -fopenmp -lgsl -lgslcblas -lm -o par_portfolio.out
	
clean :
	rm *.o *.out
