# List of Files

## Source Files
- `portfolio_lib.h` and `portfolio_lib.c`
    - Contains code that performs portfolio and asset related calculations, including:
        - Calculation of correlated normal random variables
        - Rate of return calculation for one risky asset for one month
        - Rate of return calculation for portfolio of assets for one month
    - Includes declarations of structs that are used to represent random variables and risky assets
    - `.h` file contains declarations, `.c` file contains implmentations
- `ser_portfolio.c`
    - Serial implementation of a series of portfolio performance simulations
- `par_portfolio.c`
    - Parallel implementation of a series of portfolio performance simulations
    - Contains main function
- `result_plot.py`
    - Reads the `results.txt` file and plots a histogram to visualize the distribution of the results

## Input Files
- `assets.csv`
    - Contains the distribution information of each asset
    - Contains weight of each asset in portfolio and correlation coefficient between assets

## Output Files
- `results.txt`
    - Contains results of each simulation run
    - Results are the total percentage return (in decimal format) for the entire period simulated
    - The result for each run is placed on its own line
