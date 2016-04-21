#include "portfolio_lib.h"

#include <math.h>
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_blas.h>

#ifndef GSL_H
#define GSL_H
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>
#endif

/* Creates an RNG that can be used by monte carlo simulations. The clock is 
 * used as a seed for an RNG that generates a seed for the RNG whose pointer
 * is returned. The memory pointed to by the returned pointer must be freed */
gsl_rng* initialize_rng() {

    /* MT19937 Generator passed DIEHARD statistical tests and is about
     * as fast as most other generators available in gsl, won't take very
     * long to generate a good randomized seed for the simulation */

    /* Create the RNG that will generate the seed for the monte carlo RNG */
    gsl_rng *init_rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(init_rng, clock());
    
    unsigned long int seed = gsl_rng_get(init_rng);

    gsl_rng_free(init_rng);
    init_rng = NULL;

    /* RANLUX generators have strongest proof of randomness, so that will be
     * used to generate random normal varaibles for simulations. Second 
     * generation RANLUX generators provides truly decorrelated numbers
     * at a known level of randomness. Higher luxury levels provide increased
     * decorrelation between samples as an additional safety margin */

    /* Create the RNG that will be used to generate random variables for the
     * actual monte carlo simulation */
    gsl_rng *res_rng = gsl_rng_alloc(gsl_rng_ranlxs2);
    gsl_rng_set(res_rng, seed);

    return res_rng;
}

gsl_vector* corr_norm_rvars(const int NUM_ASSETS, gsl_rng *rng,
        gsl_matrix *cholesky) {

    /* Create a vector of normal random variables */
    gsl_vector *rans = gsl_vector_alloc(NUM_ASSETS);
    for (int i = 0; i < NUM_ASSETS; i++) {
        gsl_vector_set(rans,i,gsl_ran_ugaussian(rng));
    }

    gsl_vector *corr_rans = gsl_vector_alloc(NUM_ASSETS);
    
    /* Multiply by Cholesky decomposition to generate correlated random
     * variables */
    gsl_blas_dgemv(CblasNoTrans, 1, cholesky, rans, 1, corr_rans);

    gsl_vector_free(rans);

    /* Return pointer to the vector of correlated normal random variables */
    return corr_rans;
}

double one_month_portfolio_return(risky_asset assets[],
        const int NUM_ASSETS, gsl_vector *rans) {
    double tot_ret = 0.0;
    for (int i = 0; i < NUM_ASSETS; i++) {
        risky_asset curr = assets[i];
        tot_ret += curr.port_weight * one_month_asset_return(curr.mean,
                curr.sigma, gsl_vector_get(rans,i));
    }
    return tot_ret;
}

double one_month_asset_return(double mean, double sigma, double rand_var) {
    /* 1/12 is the change in time for monthly returns */
    double delta_t = 1.0/12.0;
    return mean * delta_t + sigma * sqrt(delta_t) * rand_var;
}
