#include "portfolio_lib.h"

#include <math.h>
#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_histogram.h>

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
