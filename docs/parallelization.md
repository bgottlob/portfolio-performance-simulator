# Parallelization Approach

## General Approach
When the program is executed, a number of months (`<num_months>`) and a number of simulation runs (`<num_runs>`) are provided via command line arguments like so: `./par_portfolio.out <num_months> <num_runs>`.
For example, executing the program with this command `./par_portfolio.out 12 10000` performs 10,000 simulations where each simulation calculates the return of the portfolio over 12 months.
Each individual simulation run is executed serially. Since each simulation run does not depend on the execution of any other runs, each run is assigned to run in parallel on a thread.
This parallelization is implemented in OpenMP using the parallel for loop worksharing construct, where each iteration of the for loop is dispatched to an open thread.
The worksharing construct is applied to a loop that runs once for each simulation run to be completed. As previously mentioned, each run is executed independently from the rest, there are no loop-carried dependencies, and thus the worksharing construct can be applied.
Currently, no further parallelization occurs inside of an individual simulation run.
This approach has yielded observable parallel speedup (empirical results needed).

## Potential Bottlenecks
1. The result of each simulation run is appended to the `results.txt` file. Within the worksharing parallel for loop construct, a critical section ensures that only one thread is appending to that file at a time.
2. Plotting the histogram with matplotlib - the python program that plots the histogram of results must read the `results.txt` file serially and thus must create the frequency distribution serially. Performance of plotting should improve if the frequency distribution is created in the C program in parallel, then the bin boundaries and frequencies are provided to matplotlib.
