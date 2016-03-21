#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "portfolio_lib.h"

int main(int argc, char **argv) {

    /* TODO: Use options with command line arguments and modify makefile to
     * play nice with command line arguments */
    if (argc != 3) {
        printf("ERROR: You have not provided exactly two arguments");
        exit(1);
    }
    
    const int NUM_MONTHS = atoi(argv[1]);
    const int NUM_RUNS = atoi(argv[2]);

    // TODO: for portfolio with N assets, number of lines in file is num_assets
    int num_assets = 2;
    struct risky_asset assets[num_assets];
    double corr = 0.0;
    FILE *asset_file = fopen("assets.csv", "r");
    // TODO: Make sure all asset data is filled
    if (asset_file) {
        for (int i = 0; i < num_assets; i++) {
            fscanf(asset_file, "%lg,%lg,%lg", &assets[i].mean, &assets[i].sigma,
                    &assets[i].port_weight);
        }
        fscanf(asset_file, "%lg", &corr);
    } else {
        printf("ERROR: no asset file provided");
        exit(1);
    }

    double total_weight = 0;
    for (int i = 0; i < num_assets; i++) {
        total_weight += assets[i].port_weight;
    }
    if (total_weight != 1.0) {
        printf("ERROR: weights of assets do not total 100%%");
        exit(1);
    }

    //TODO: Get the number of runs and months from the command line args
    
    /* This file will contain the final porfolio returns for all runs */
    FILE *results_file = fopen("results.txt","w");

    /* Each run of this loop is one simulation of portfolio return */
#pragma omp parallel for
    for (int run = 0; run < NUM_RUNS; run++) { 

        double total_return = 0;

        /* For debugging purposes */
        /*if (omp_get_thread_num() == 0) {
            printf("Number of threads: %d\n", omp_get_num_threads());
        }*/

        /* Calculate the monthly portfolio return
         * and add to yearly return accumulator */
        for (int month = 1; month <= NUM_MONTHS; month++) {
            struct corr_norm rand_vars = two_corr_norm_rvars(corr);
            double month_ret = one_month_portfolio_return(assets[0], assets[1],
                    rand_vars.var1, rand_vars.var2);
            total_return += month_ret;
        }


        if(results_file) {
            #pragma omp critical
            {
                fprintf(results_file, "%f\n", total_return);
            }
        }

        
        /* printf("Total return: %f%%\n", total_return * 100); */

    }

    exit(0);
}
