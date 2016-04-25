# Compile the shared library of portfolio functions
objs/portfolio_lib.o : src/portfolio_lib.h src/portfolio_lib.c
	gcc -c -g src/portfolio_lib.c -o objs/portfolio_lib.o

objs/driver_lib.o : src/driver_lib.h src/driver_lib.c
	gcc -c -g src/driver_lib.c -o objs/driver_lib.o

# Compile serial implementation
objs/ser_portfolio.o : src/portfolio.c
	gcc -c src/portfolio.c -o objs/ser_portfolio.o

# Compile parallel implementation
objs/par_portfolio.o : src/portfolio.c
	gcc -c -g -fopenmp src/portfolio.c -o objs/par_portfolio.o

# Links the libraries with both serial and parallel implementations to create executable files
compile : objs/par_portfolio.o objs/ser_portfolio.o objs/portfolio_lib.o objs/driver_lib.o
	gcc objs/portfolio_lib.o objs/driver_lib.o objs/ser_portfolio.o -g -lcurl -lgsl -lgslcblas -lm -o ser_portfolio.out
	gcc objs/portfolio_lib.o objs/driver_lib.o objs/par_portfolio.o -g -lcurl -fopenmp -lgsl -lgslcblas -lm -o par_portfolio.out
	
clean :
	rm -r *.out *.dSYM objs/* data/prices/*
