# Monte Carlo Simulation - Portfolio Performance

## Compiling, Linking, and Running
To compile both parallel and serial implementations, execute this command: `make compile`

To run the serial code, execute this command: `./ser_portfolio.out <num_months> <num_runs>`

To run the parallel code, execute this command: `./par_portfolio <num_months> <num_runs>`

In order to run the parallel code with multiple threads, be sure to set the `OMP_NUM_THREADS` environment variable to the number of threads that you would like to use. If this environment variable is not set, you will not see any parallel speedup

To see the histogram of the most recently run set of simulations, execute this command: `python result_plot.py`

## Dependencies
- `gcc` with OpenMP and GSL
    - Note that on Mac OS X, the gcc command defaults to the LLVM compiler, which does not support OpenMP
    - To add support for OpenMP, download GNU gcc from Homebrew and make GNU gcc your default gcc compiler
- `python`
- `matplotlib`
