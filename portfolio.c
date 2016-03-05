#include <stdio.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_histogram.h>
#include <time.h>
#include <math.h>

#define NUM_RUNS 30
#define NUM_MONTHS 12

// TODO: Standardize naming conventions of code

/* Stores a pair of correlated random normal variables */
struct corrNorm {
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

/* Returns a struct containing two correlated, normally
 * distributed random variables */
struct corrNorm twoCorrNormRVars(double corr);

/* Calculates the return of a portfolio for one month given distribution
 * information for two risky assets and two correlated random variables */
double month_port_return(struct risky_asset asset1, struct risky_asset asset2,
        double randVar1, double randVar2);

/* Calculates the return of a single risky asset given distribution
 * information for that asset */
double month_asset_return(double mean, double sigma, double randVar);

int main(void) {

    // TODO: Feed this information in from a file or command line args
    struct risky_asset stock1;
    stock1.mean = 0.12;
    stock1.sigma = 0.1;
    stock1.port_weight = 0.3;

    struct risky_asset stock2;
    stock2.mean = 0.22;
    stock2.sigma = 0.15;
    stock2.port_weight = 0.7;

    double corr = 0.2;

    // TODO: Ensure that weights of assets add up to 100%
    
    /*
     * Create a histogram to keep track of the simulated data
     * Code no longer needed since histogram will be taken car of by
     * matplotlib. Leaving this comment here to be version controlled
     * in case this code is needed for reference later
    int numBins = 19;
    double expRetPort = 0.19;
    double sigmaPort = 0.11;
    gsl_histogram *h = gsl_histogram_alloc(numBins);
    double range[20] = { -.04, -.02, 0, .02, .04, .06, .08, .1, .12, .14, .16, .18, .20, .22, .24, .26, .28, .30, .32, .34};
    gsl_histogram_set_ranges(h, range, 20);
    */

    //TODO: Get the number of runs and months from the command line args or file
    
    /* Each run of this loop is one simulation of portfolio return */
    for (int run = 0; run < NUM_RUNS; run++) { 

        double total_return = 0;

        /* Calculate the monthly portfolio return
         * and add to yearly return accumulator */
        for (int mon = 1; mon <= NUM_MONTHS; mon++) {
            struct corrNorm randVars = twoCorrNormRVars(corr);
            double month_ret = month_port_return(stock1, stock2,
                    randVars.var1, randVars.var2);
            total_return += month_ret;
        }

        /*
         * Code no longer necessary, version controlling for future reference
         gsl_histogram_increment(h, yearlyRet);
         */

        printf("\nTotal return: %f%%\n", total_return * 100);
    }

    /*
     * Code no longer necessary, version controlling for future
     * reference.
    TODO: Find out what happened to -.02 to 0 and 0 to .02 bin
    for (int binNum = 0; binNum < numBins; binNum++) {
        double min, max;
        gsl_histogram_get_range(h, binNum, &min, &max);
        if (min && max)
            printf("Bin from %f to %f: %d\n", min, max, (int)gsl_histogram_get(h, binNum));
    }
    gsl_histogram_free(h);
    */

    exit(0);
}

struct corrNorm twoCorrNormRVars(double corr) {

    gsl_rng *r = gsl_rng_alloc(gsl_rng_taus2);
    // TODO: Randomize the seed properly rather than using the clock
    clock_t seed = clock();
    gsl_rng_set(r, seed);
    double corrVar1 = gsl_ran_ugaussian(r);

    /* Implementing formula for finding two correlated random variables */
    double corrVar2 = corr * corrVar1 +
        sqrt(1-pow(corr,2)) * gsl_ran_ugaussian(r);

    gsl_rng_free(r);

    struct corrNorm res;
    res.var1 = corrVar1;
    res.var2 = corrVar2;
    
    return res;
}

double month_port_return(struct risky_asset asset1, struct risky_asset asset2,
        double randVar1, double randVar2) {
    return asset1.port_weight *
            month_asset_return(asset1.mean, asset1.sigma, randVar1) +
            asset2.port_weight *
            month_asset_return(asset2.mean, asset2.sigma, randVar2);
}

double month_asset_return(double mean, double sigma, double randVar) {
    /* 1/12 is the change in time for monthly returns;
     * static because it only needs to be calculated once for all
     * runs of this function */
    static double delta_t = 1.0/12.0;
    return mean * delta_t + sigma * sqrt(delta_t) * randVar;
}
