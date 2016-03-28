#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "portfolio_lib.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

int main(int argc, char **argv) {

    /* TODO: Use options with command line arguments and modify makefile to
     * play nice with command line arguments */
    if (argc != 3) {
        printf("ERROR: You have not provided exactly two arguments");
        exit(1);
    }
    
    const int NUM_MONTHS = atoi(argv[1]);
    const int NUM_RUNS = atoi(argv[2]);
    const int NUM_ASSETS;

    gsl_matrix *varcovar;

    FILE *vc_file = fopen("varcovar.csv", "r");
    if (vc_file) {

        /* Get the number of assets in the portfolio as first line in file */
        fscanf(vc_file, "%d", &NUM_ASSETS);

        /* Create matrix with size NUM_ASSETS by NUM_ASSETS */
        varcovar = gsl_matrix_alloc(NUM_ASSETS, NUM_ASSETS);

        /* Read all numbers from file and place into matrix */
        for (int i = 0; i < NUM_ASSETS; i++) {
            for (int j = 0; j < NUM_ASSETS; j++) {
                double curr; 
                fscanf(vc_file, "%lf,", &curr);
                gsl_matrix_set(varcovar, i, j, curr);
            } 
        }
    } else {
        printf("ERROR: no variance-covariance matrix file provided");
        exit(1);
    }

    // TODO: Must free varcovar's memory!

    struct risky_asset assets[NUM_ASSETS];

    FILE *asset_file = fopen("assets.csv", "r");
    // TODO: Make sure all asset data is filled
    if (asset_file) {
        for (int i = 0; i < NUM_ASSETS; i++) {
            fscanf(asset_file, "%lg,%lg,%lg", &assets[i].mean, &assets[i].sigma,
                    &assets[i].port_weight);
        }
    } else {
        printf("ERROR: no asset file provided\n");
        exit(1);
    }

    double total_weight = 0;
    for (int i = 0; i < NUM_ASSETS; i++) {
        total_weight += assets[i].port_weight;
    }
    if (total_weight != 1.0) {
        printf("ERROR: weights of assets do not total 100%%\n");
        exit(1);
    }

    // For debugging purposes
    for (int i = 0; i < NUM_ASSETS; i++) {
        for (int j = 0; j < NUM_ASSETS; j++) {
            printf("%6.3lg |", gsl_matrix_get(varcovar,i,j));
        }
        printf("\n");
    }
    // --------------


    // TODO: Extra Cholesky matrix is not needed, varcovar ptr can be used
    /* Find Cholesky decomposition for use in MC simulation */
    gsl_matrix *cholesky = gsl_matrix_alloc(NUM_ASSETS, NUM_ASSETS);
    gsl_matrix_memcpy(cholesky, varcovar);
    gsl_linalg_cholesky_decomp(cholesky);

    /* Make the Cholesky decomposition matrix L lower triangular */
    for (int i = 0; i < NUM_ASSETS; i++) {
        for (int j = i+1; j < NUM_ASSETS; j++) {
            gsl_matrix_set(cholesky,i,j,0.0);
        }
    }

    // For debugging purposes
    printf("\nCholesky Decomp:\n");
    for (int i = 0; i < NUM_ASSETS; i++) {
        for (int j = 0; j < NUM_ASSETS; j++) {
            printf("%6.3lg |", gsl_matrix_get(cholesky,i,j));
        }
        printf("\n");
    }
    // ------------

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

        /* Create a random number generator */
        gsl_rng *rng = initialize_rng();

        /* Calculate the monthly portfolio return
         * and add to yearly return accumulator */
        gsl_vector *rans;
        for (int month = 1; month <= NUM_MONTHS; month++) {
            rans = corr_norm_rvars(NUM_ASSETS, rng, cholesky);
            double month_ret = one_month_portfolio_return(assets, NUM_ASSETS,
                    rans);
            total_return += month_ret;
        }

        /* Free memory taken by random number generator */
        gsl_rng_free(rng);
        gsl_vector_free(rans);

        if(results_file) {
            #pragma omp critical
            {
                fprintf(results_file, "%f\n", total_return);
            }
        }

        
        /* printf("Total return: %f%%\n", total_return * 100); */

    }

    gsl_matrix_free(cholesky);

    exit(0);
}
