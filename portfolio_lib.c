#include "portfolio_lib.h"

#include <math.h>
#include <time.h>
#include <gsl/gsl_randist.h>

#ifndef GSL_RNG_H
#define GSL_RNG_H
#include <gsl/gsl_rng.h>
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

struct corr_norm two_corr_norm_rvars(double corr, gsl_rng *sim_rng) {

    double corr_var1 = gsl_ran_ugaussian(sim_rng);

    /* Implementing formula for finding two correlated random variables */
    double corr_var2 = corr * corr_var1 +
        sqrt(1-pow(corr,2)) * gsl_ran_ugaussian(sim_rng);

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
