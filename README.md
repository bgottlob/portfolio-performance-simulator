# Monte Carlo Simulation - Portfolio Performance

## Deps
- gsl
- libcurl
- openmp
- pyplot

## Snafus
- Installing ATLAS on Ubuntu
- BLAS function for multiplying matrix by vector is returning NANs

## Prerequisites
Ensure that you have GNU `gcc` installed so you can use OpenMP.
Ensure that the `gcc` command defaults to 
Install GNU Scientific Library (gsl). Ensure that your library include and linking paths for gcc contain the path to gsl.
Create two subdirectories called `data` and `objs`. The simulations and compilation process needs to create files within these directories. You can create these subdirectories by running `mkdir data` and `mkdir objs`
Be sure to be connected to the Internet when running. Stock data is retrieved from Yahoo! Finance.

## Compiling, Linking, and Running
To compile both parallel and serial implementations, execute this command: `make compile`

To run the serial code, execute this command: `./ser_portfolio.out <num_months> <num_runs> <portfolio_name> [<initial_seed>]`

To run the parallel code, execute this command: `./par_portfolio <num_months> <num_runs> <portfolio_name> [<initial_seed>]`

In order to run the parallel code with multiple threads, be sure to set the `OMP_NUM_THREADS` environment variable to the number of threads that you would like to use. If this environment variable is not set, you will not see parallel speedup. To set the number of threads to `4`, use this command `export OMP_NUM_THREADS=4`

To see the histogram of the most recently run set of simulations, execute this command: `python src/plot.py <chart-title>`

## Example Test Run
Once you have compiled, you can run 10000 simulations in parallel for 2 years with the MKT portfolio (details given by the `data/tickersMKT.csv` and `weightsMKT.csv` files) with an initial random number generator seed of 0 using this command `./par_portfolio.out 24 10000 MKT 0`
