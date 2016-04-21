#include <stdio.h>
#include <stdlib.h>
#include "portfolio_lib.h"
#include "driver_lib.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_statistics_double.h>
#include <string.h>

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("ERROR: You have not provided exactly two arguments\n");
        exit(1);
    }
    
    const int NUM_MONTHS = atoi(argv[1]);
    const int NUM_RUNS = atoi(argv[2]);
    size_t NUM_ASSETS = 0;

    char **ticks = read_ticker_file("data/tickers.csv", &NUM_ASSETS);

    ret_data *dataset = malloc(NUM_ASSETS * sizeof(ret_data));
    risky_asset *assets = malloc(NUM_ASSETS *sizeof(risky_asset));

    time_t t = time(NULL);
    struct tm curr_time = *localtime(&t);

    for (int i = 0; i < NUM_ASSETS; i++) {
        dataset[i].data = read_price_file(
                get_stock_file(ticks[i], curr_time, 6), &dataset[i].size);
        assets[i].ticker = malloc((strlen(ticks[i]) + 1) * sizeof(char));

        /* Actually copy the string instead of setting pointers equal so that 
         * the ticks array can be freed */
        strcpy(assets[i].ticker, ticks[i]);
        assets[i].mean = gsl_stats_mean(dataset[i].data,1,dataset[i].size);
        assets[i].sigma = gsl_stats_sd(dataset[i].data,1,dataset[i].size);

        /* For now, set all stock weights to be equal */
        assets[i].port_weight = 1.0/NUM_ASSETS;
    }
    free(ticks);

    gsl_matrix *varcovar = calculate_varcovar(dataset, NUM_ASSETS);
    free(dataset);
    perform_cholesky(varcovar, NUM_ASSETS);
    gsl_matrix *cholesky = varcovar;

    printf("Finished crunching numbers and getting data for stock returns\nBeginning simulations\n");

    /* This file will contain the final porfolio returns for all runs */
    FILE *results_file = fopen("data/results.txt","w");
    if(results_file) {
        /* Each run of this loop is one simulation of portfolio return */
        for (int run = 0; run < NUM_RUNS; run++) { 
            double total_return = 0;
            gsl_rng *rng = initialize_rng();

            /* Calculate the monthly portfolio return
             * and add to yearly return accumulator */
            gsl_vector *rans;
            for (int month = 1; month <= NUM_MONTHS; month++) {
                rans = corr_norm_rvars(NUM_ASSETS, rng, cholesky);
                double month_ret = one_month_portfolio_return(assets,
                        NUM_ASSETS, rans);
                total_return += month_ret;
            }
            gsl_rng_free(rng);
            gsl_vector_free(rans);

            fprintf(results_file, "%f\n", total_return);
            /* printf("Total return: %f%%\n", total_return * 100); */
        }
    }
    gsl_matrix_free(cholesky);
    exit(0);

}
