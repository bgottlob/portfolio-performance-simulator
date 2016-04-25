#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>

#ifndef PORTFOLIO_LIB_H
#define PORTFOLIO_LIB_H
/* Stores statistics information on the annual distribution of returns 
 * of a single risky asset along with its ticker and
 * weight within the portfolio */
struct {
    char *ticker;
    double mean;
    double sigma; /* The standard deviation of returns */
    double port_weight;
} typedef risky_asset;

gsl_rng* initialize_rng();

gsl_rng* initialize_rng_with_seed(unsigned long seed);

void perform_cholesky(gsl_matrix *matrix, const int NUM_ASSETS);

/* Returns a pointer to a vector containing correlated, normally distributed 
 * random variables given the Cholesky decomposition of the assets' 
 * variance-covariance matrix */
gsl_vector* corr_norm_rvars(const int NUM_ASSETS, gsl_rng *rng,
        gsl_matrix *cholesky);

/* Calculates the return of a portfolio for one month given distribution
 * information for two risky assets and two correlated random variables */
double one_month_portfolio_return(risky_asset assets[],
        const int NUM_ASSETS, gsl_vector *rans);

/* Calculates the return of a single risky asset given distribution
 * information for that asset and a random variable for that month */
double one_month_asset_return(double mean, double sigma, double rand_var);
#endif
