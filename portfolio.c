#include <stdio.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_histogram.h>
#include <time.h>
#include <math.h>

struct corrNorm {
    double var1;
    double var2;
};

struct risky_asset {
    double mean;
    double sigma;
    double port_weight;
};

// Returns a struct containing two correlated, normally distributed random variables
struct corrNorm twoCorrNormRVars(double corr);
double month_port_return(struct risky_asset asset1, struct risky_asset asset2, double randVar1, double randVar2);
double month_asset_return(double mean, double sigma, double randVar);

int main(void) {

    struct risky_asset stock1;
    stock1.mean = 0.12;
    stock1.sigma = 0.1;
    stock1.port_weight = 0.3;

    struct risky_asset stock2;
    stock2.mean = 0.22;
    stock2.sigma = 0.15;
    stock2.port_weight = 0.7;

    double corr = 0.2;

    // Create a histogram to keep track of the simulated data
    int numBins = 19;
    double expRetPort = 0.19;
    double sigmaPort = 0.11;
    gsl_histogram *h = gsl_histogram_alloc(numBins);
    double range[20] = { -.04, -.02, 0, .02, .04, .06, .08, .1, .12, .14, .16, .18, .20, .22, .24, .26, .28, .30, .32, .34};
    gsl_histogram_set_ranges(h, range, 20);

    int numRuns = 100;
    for (int run = 0; run < numRuns; run++) { 
        int numMonths = 12;
        double yearlyRet = 0;
        for (int mon = 1; mon <= numMonths; mon++) {
            struct corrNorm randVars = twoCorrNormRVars(corr);
            double month_ret = month_port_return(stock1, stock2, randVars.var1, randVars.var2);
            yearlyRet += month_ret;
            //printf("Month %d: %lg\n", mon, month_ret);
        }
        gsl_histogram_increment(h, yearlyRet);
        //printf("\nYearly return: %f%%\n", yearlyRet * 100);
    }

    for (int binNum = 0; binNum < numBins; binNum++) {
        double min, max;
        gsl_histogram_get_range(h, binNum, &min, &max);
        if (min && max)
            printf("Bin from %f to %f: %d\n", min, max, (int)gsl_histogram_get(h, binNum));
    }

    gsl_histogram_fprintf(stdout, h, "%g", "%g");

    gsl_histogram_free(h);

    exit(0);
}

struct corrNorm twoCorrNormRVars(double corr) {

    gsl_rng *r = gsl_rng_alloc(gsl_rng_taus2);
    // TODO: Randomize the seed rather than using the clock
    clock_t seed = clock();
    //printf("Seeding generator with: %d\n", (int)seed);
    gsl_rng_set(r, seed);
    double corrVar1 = gsl_ran_ugaussian(r);

    // Using formula from Fin Modeling book to generate a second correlated random variable
    double corrVar2 = corr * corrVar1 + sqrt(1-pow(corr,2))*gsl_ran_ugaussian(r);

    gsl_rng_free(r);

    struct corrNorm res;
    res.var1 = corrVar1;
    res.var2 = corrVar2;
    
    return res;
}

double month_port_return(struct risky_asset asset1, struct risky_asset asset2, double randVar1, double randVar2) {
    return asset1.port_weight * month_asset_return(asset1.mean, asset1.sigma, randVar1) +
        asset2.port_weight * month_asset_return(asset1.mean, asset2.sigma, randVar2);
}

double month_asset_return(double mean, double sigma, double randVar) {
    // 1/12 is delta t for monthly returns
    double delta_t = 1.0/12.0;
    return mean * delta_t + sigma * sqrt(delta_t) * randVar;
}
