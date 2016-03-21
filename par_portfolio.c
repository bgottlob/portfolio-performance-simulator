#include <stdio.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_histogram.h>
#include <time.h>
#include <math.h>
#include <omp.h>

/* Stores a pair of correlated random normal variables */
struct corr_norm {
    double var1;
    double var2;
};

/* Stores information on the annual distribution of returns of a single risky
 * asset along with its weight within the portfolio */
struct risky_asset {
    double mean;
    double sigma;
    double port_weight;
};

/* Returns a struct containing two correlated, normally distributed random
 * variables given the correlation coeffcient between them*/
struct corr_norm two_corr_norm_rvars(double corr);

/* Calculates the return of a portfolio for one month given distribution
 * information for two risky assets and two correlated random variables */
double one_month_portfolio_return(struct risky_asset asset1,
        struct risky_asset asset2, double rand_var1, double rand_var2);

/* Calculates the return of a single risky asset given distribution
 * information for that asset and a random variable for that month */
double one_month_asset_return(double mean, double sigma, double rand_var);

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

struct corr_norm two_corr_norm_rvars(double corr) {

    gsl_rng *r = gsl_rng_alloc(gsl_rng_taus2);
    // TODO: Randomize the seed properly rather than using the clock
    clock_t seed = clock();
    gsl_rng_set(r, seed);
    double corr_var1 = gsl_ran_ugaussian(r);

    /* Implementing formula for finding two correlated random variables */
    double corr_var2 = corr * corr_var1 +
        sqrt(1-pow(corr,2)) * gsl_ran_ugaussian(r);

    gsl_rng_free(r);

    struct corr_norm res;
    res.var1 = corr_var1;
    res.var2 = corr_var2;
    
    return res;
}

double one_month_portfolio_return(struct risky_asset asset1,
        struct risky_asset asset2, double rand_var1, double rand_var2) {
    return asset1.port_weight *
            one_month_asset_return(asset1.mean, asset1.sigma, rand_var1) +
            asset2.port_weight *
            one_month_asset_return(asset2.mean, asset2.sigma, rand_var2);
}

double one_month_asset_return(double mean, double sigma, double rand_var) {
    /* 1/12 is the change in time for monthly returns;
     * static because it only needs to be calculated once for all
     * runs of this function */
    static double delta_t = 1.0/12.0;
    return mean * delta_t + sigma * sqrt(delta_t) * rand_var;
}
