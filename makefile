# Compile the shared library of portfolio functions
portfolio_lib.o : src/portfolio_lib.h src/portfolio_lib.c
	gcc -c -g src/portfolio_lib.c -o portfolio_lib.o

driver_lib.o : src/driver_lib.h src/driver_lib.c
	gcc -c -g src/driver_lib.c -o driver_lib.o

# Compile serial implementation
ser_portfolio.o : src/ser_portfolio.c
	gcc -c src/ser_portfolio.c -o ser_portfolio.o

# Compile parallel implementation
par_portfolio.o : src/par_portfolio.c
	gcc -c -g -fopenmp src/par_portfolio.c -o par_portfolio.o

# Links the libraries with both serial and parallel implementations to create executable files
compile : par_portfolio.o ser_portfolio.o portfolio_lib.o driver_lib.o
	gcc portfolio_lib.o driver_lib.o ser_portfolio.o -g -lcurl -lgsl -lgslcblas -lm -o ser_portfolio.out
	gcc portfolio_lib.o driver_lib.o par_portfolio.o -g -lcurl -fopenmp -lgsl -lgslcblas -lm -o par_portfolio.out
	
clean :
	rm -r *.o *.out *.dSYM
