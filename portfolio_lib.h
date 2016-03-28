#ifndef GSL_H
#define GSL_H
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>
#endif

#ifndef PORTFOLIO_DEFS_H
#define PORTFOLIO_DEFS_H

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

gsl_rng* initialize_rng();

/* Returns a pointer to a vector containing correlated, normally distributed 
 * random variables given the Cholesky decomposition of the assets' 
 * variance-covariance matrix */
gsl_vector* corr_norm_rvars(const int NUM_ASSETS, gsl_rng *rng,
        gsl_matrix *cholesky);

/* Calculates the return of a portfolio for one month given distribution
 * information for two risky assets and two correlated random variables */
double one_month_portfolio_return(struct risky_asset assets[],
        const int NUM_ASSETS, gsl_vector *rans);

/* Calculates the return of a single risky asset given distribution
 * information for that asset and a random variable for that month */
double one_month_asset_return(double mean, double sigma, double rand_var);

#endif
