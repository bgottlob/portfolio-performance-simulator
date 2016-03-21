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

#endif
