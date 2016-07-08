# Monte Carlo Simulation - Portfolio Performance
This project implements Monte Carlo methods to simulate the performance of a portfolio of investments. Yahoo! Finance is queried for monthly prices of stocks, bonds, funds, and other securities. The implementation of the simulations is parallelized for better performance using OpenMP's multithreading model. To read in-depth about the finance, math, and computer science concepts utilized in the project see [my poster](https://dl.dropboxusercontent.com/u/15687514/monte-carlo-project/mc_performance_poster.pdf). For even more details, see [my research paper](https://dl.dropboxusercontent.com/u/15687514/monte-carlo-project/mc_performance_paper.pdf).

## Dependencies
Before attempting to build this project, first be sure to install the following dependencies:
- [GNU Scientific Library (GSL)](https://www.gnu.org/software/gsl/)
- [Libcurl](https://curl.haxx.se/)
- [OpenMP](http://openmp.org/wp/)
- [pyplot (part of matplotlib)](http://matplotlib.org/api/pyplot_summary.html)

## Prerequisites
The GNU `gcc` compiler comes pre-equipped with OpenMP. On Mac OS X, `gcc` is a symlink to `clang` by default. `clang` does not come-prequipped with OpenMP. You can download `clang`'s OpenMP implementation through the [`clang-omp` Homebrew formula](http://brewformulas.org/ClangOmp). However, please be aware that this project has only been tested using GNU `gcc`. GNU `gcc` can be installed on Mac OS X using the [`gcc` Homebrew formula](http://brewformulas.org/Gcc) and the `--without-multilib` option on install like so: `brew install gcc --without-multilib`. Without that option, OpenMP support will not be installed. In order to use my `Makefile` without any modifications on Mac OS X with GNU `gcc`, you must create a symlink to run the Homebrew-installed `gcc`. See [this entry on StackOverflow](http://stackoverflow.com/questions/29057437/compile-openmp-programs-with-gcc-compiler-on-os-x-yosemite) for more details.


## Compiling
Before compiling, create two subdirectories called `data` and `objs`. The simulations and compilation process needs to create files within these directories. You can create these subdirectories by running `mkdir data` and `mkdir objs`

To compile both parallel and serial implementations, execute this command: `make compile`

## Running
Not all `omp parallel` pragmas specify a number of threads to use. For those pragmas, thenumber of threads specified by the `OMP_NUM_THREADS` environment variable. Be sure to set this environment variable before running the parallel executable so that the desired number of parallel threads will be used. To set this environment variable to `4`, for example, use the following command: `export OMP_NUM_THREADS=4`

To run the serial code, execute this command: `./ser_portfolio.out <num_months> <num_runs> <portfolio_name> [<initial_seed>]`

To run the parallel code, execute this command: `./par_portfolio <num_months> <num_runs> <portfolio_name> [<initial_seed>]`

To see the histogram of the most recently run set of simulations, execute this command: `python src/plot.py <chart-title>`

**NOTE:** Be sure to be connected to the Internet when running. Stock data is retrieved from Yahoo! Finance. A command line option to skip the remote retrieval and read files locally is coming soon.

### Example Test Run
Once you have compiled, you can run 10000 simulations in parallel for 2 years with the MKT portfolio (details given by the `data/tickersMKT.csv` and `weightsMKT.csv` files) with an initial random number generator seed of 4 using this command `./par_portfolio.out 24 10000 MKT 4`. To perform this simulation and plot the results when finished, run this command `./par_portfolio.out 24 10000 MKT 4 && python src/plot.py`
