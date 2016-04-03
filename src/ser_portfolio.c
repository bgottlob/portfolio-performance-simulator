#include <stdio.h>
#include <stdlib.h>
#include "portfolio_lib.h"
#include "driver_lib.h"

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("ERROR: You have not provided exactly two arguments\n");
        exit(1);
    }
    
    const int NUM_MONTHS = atoi(argv[1]);
    const int NUM_RUNS = atoi(argv[2]);
    const int NUM_ASSETS;

    gsl_matrix *varcovar = varcovar_from_file("data/varcovar.csv", &NUM_ASSETS);
    struct risky_asset *assets = assets_from_file("data/assets.csv", NUM_ASSETS);
    perform_cholesky(varcovar, NUM_ASSETS);
    gsl_matrix *cholesky = varcovar;

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
