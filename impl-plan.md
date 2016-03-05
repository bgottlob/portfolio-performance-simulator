## Implementation Plan
1. Create program to simulate performance of two-stock portfolio with clock seeding the random number generator
    - Givens - correlation, means, standard devs
        - Take this input from a file
    - Use matplotlib to plot histograms
        - Write makefile to automatically pipe output into matplotlib
    - Document completely throughout development
1. Parallelize this two-stock simulation
    - Utilize specific topics covered in OpenMP tutorials
    - Document the ways in which it was parallelized
1. Review approaches to randomizing seed for Monte Carlo
    - Document decision and rationale based on good sources
1. Run experiments with parallel Monte Carlo simulation
    - Utilize profiling tools
    - Document results for paper
    - Try to find ways to improve performance
    - Identify bottlenecks
1. Write program to calculate correlation, means, standard devs of stocks
    - Given historical returns data
1. Parallelize calculation of correlation, means, standard deviation if possible
1. Run simulations for historical data and see how accurate the simulation is
1. Generalize to N assets in portfolio - need Cholesky decomposition
1. Find ways to apply to retirement problem, answer questions about different stocks, etc.
