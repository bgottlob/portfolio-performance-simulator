#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>
#include <sys/time.h>

#include "portfolio_lib.h"

#include "driver_lib.h"


#include <gsl/gsl_matrix.h>
#include <string.h>
#include <gsl/gsl_statistics_double.h>

int main(int argc, char **argv) {

    unsigned long *seed_ptr = NULL;

    /* Idea: Inital seed could be randomly generated */
    unsigned long seed = 0;

    if (argc < 4 || argc > 5) {
        printf("ERROR: You have not provided exactly three or"
                "exactly four arguments\n");
        exit(1);
    } else if (argc == 5) {
        seed = strtoul(argv[4], NULL, 10); 
        seed_ptr = &seed;
    }
    
    const int NUM_MONTHS = atoi(argv[1]);
    const int NUM_RUNS = atoi(argv[2]);
    const char *PORT_SUFF = argv[3];
    size_t NUM_ASSETS = 0;

    char *subdir = "data";
    char *tickers_name = "tickers";
    char *weights_name = "weights";
    char *extension = "csv";
    char ticker_filename[strlen(subdir) + strlen(tickers_name)
        + strlen(PORT_SUFF) + strlen(extension) + 3];
    char weights_filename[strlen(subdir) + strlen(weights_name)
        + strlen(PORT_SUFF) + strlen(extension) + 3];

    snprintf(ticker_filename, sizeof(ticker_filename), "%s/%s%s.%s",
            subdir, tickers_name, PORT_SUFF, extension);
    snprintf(weights_filename, sizeof(weights_filename), "%s/%s%s.%s",
            subdir, weights_name, PORT_SUFF, extension);

    char **ticks = read_ticker_file(ticker_filename, &NUM_ASSETS);
    double *weights = read_weight_file(weights_filename, NUM_ASSETS);

    ret_data *dataset = malloc(NUM_ASSETS * sizeof(ret_data));
    risky_asset *assets = malloc(NUM_ASSETS *sizeof(risky_asset));

    time_t t = time(NULL);
    struct tm curr_time = *localtime(&t);
    
    struct timeval retrieval_begin, retrieval_end, sim_begin, sim_end;
    double retrieval_time, sim_time;
    gettimeofday(&retrieval_begin, NULL);

    int get_from_yahoo = 0;
    #pragma omp parallel for num_threads(NUM_ASSETS)
    for (int i = 0; i < NUM_ASSETS; i++) {

        /* For debugging purposes
        #if defined(_OPENMP)
            printf("thread num: %d\n", omp_get_num_threads());
        #endif
        */

        /*char *price_filename = get_stock_file(ticks[i], curr_time, 6);
        dataset[i].data = read_price_file(price_filename, &dataset[i].size);*/

        /* TODO: Skip download of data to use
         * data that has already been retrieved */
        char *price_subdir = "data/prices";
        char *price_extension = "csv";
        size_t price_fname_chars = strlen(ticks[i]) + strlen(price_subdir) +
                        strlen(price_extension) + 3;
        char price_filename[price_fname_chars];
        snprintf(price_filename, price_fname_chars, "%s/%s.%s",
                price_subdir, ticks[i], price_extension);
        dataset[i].data = read_price_file(price_filename, &dataset[i].size);


        assets[i].ticker = malloc((strlen(ticks[i]) + 1) * sizeof(char));

        /* Actually copy the string instead of setting pointers equal so that 
         * the ticks array can be freed */
        strcpy(assets[i].ticker, ticks[i]);
        assets[i].mean = gsl_stats_mean(dataset[i].data,1,dataset[i].size) * 12;
        assets[i].sigma = gsl_stats_sd(dataset[i].data,1,dataset[i].size) * 12;

        /* For now, set all stock weights to be equal
        assets[i].port_weight = 1.0/NUM_ASSETS;*/

        assets[i].port_weight = weights[i];

        //printf("%s\nmu: %lg\nsig: %lg\n", assets[i].ticker, assets[i].mean, assets[i].sigma);
    }
    gettimeofday(&retrieval_end, NULL);
    retrieval_time = retrieval_end.tv_usec - retrieval_begin.tv_usec;

    for (int i = 0; i < NUM_ASSETS; i++)
        free(ticks[i]);
    free(ticks);
    free(weights);

    gsl_matrix *varcovar = calculate_varcovar(dataset, NUM_ASSETS);
    free(dataset);
    perform_cholesky(varcovar, NUM_ASSETS);
    gsl_matrix *cholesky = varcovar;
    
    printf("Finished crunching numbers and getting data for stock returns\n"
            "Beginning simulations\n");

    /* This file will contain the final porfolio returns for all runs */
    FILE *results_file = fopen("data/results.txt","w");
    /* The results that will be displayed on histogram */
    double total_rets[NUM_RUNS];
    gettimeofday(&sim_begin, NULL);
    time_t start = time(NULL), end;
    unsigned long init_seed = seed;
    int num_sim_threads = 1;
    #if defined(_OPENMP)
        num_sim_threads = omp_get_max_threads();
    #endif
    if(results_file) {
        /* Each run of this loop is one simulation of portfolio return */
        #pragma omp parallel for
        for (int run = 0; run < NUM_RUNS; run++) { 
            double total_return = 0;

            gsl_rng *rng;
            if (seed_ptr) {
                #pragma omp critical
                {
                    rng = initialize_rng_with_seed(seed);
                    /* When seed hits max unsigned long it will wrap to 0 */
                    seed++; 
                }
            } else {
                rng = initialize_rng();
            }


            /* Calculate the monthly portfolio return
             * and add to yearly return accumulator */
            gsl_vector *rans;
            for (int month = 1; month <= NUM_MONTHS; month++) {
                rans = corr_norm_rvars(NUM_ASSETS, rng, cholesky);
                double month_ret = one_month_portfolio_return(assets,
                        NUM_ASSETS, rans);
                total_return += month_ret;
            }
            gsl_rng_free(rng);
            gsl_vector_free(rans);

            total_rets[run] = total_return;

            //printf("Total return: %f%%\n", total_return * 100);
        }
        gettimeofday(&sim_end, NULL);

        retrieval_time = retrieval_end.tv_sec - retrieval_begin.tv_sec;
        retrieval_time += ((double)(retrieval_end.tv_usec - retrieval_begin.tv_usec)) / 1000000.0;

        sim_time = sim_end.tv_sec - sim_begin.tv_sec;
        sim_time += ((double)(sim_end.tv_usec - sim_begin.tv_usec)) / 1000000.0;
        printf("Ret time: %lg\nSim time: %lg\n", retrieval_time, sim_time);

        for (int i = 0; i < NUM_RUNS; i++) {
            fprintf(results_file, "%lg\n", total_rets[i] * 100);
        }
        fclose(results_file);

        double res_mean = gsl_stats_mean(total_rets,1,NUM_RUNS) * 100;
        double res_std = gsl_stats_sd(total_rets,1,NUM_RUNS)* 100;

        printf("Mean of annual returns: %lg%%\n"
                "Standard dev of annual returns: %lg%%\n",
                res_mean, res_std);

        printf("seed: %lu", init_seed);
        FILE *test_file = fopen("data/test_res_divers.csv", "a");
        if (test_file) {
           fprintf(test_file, "%d,%d,%d,%d,%d,%lg,%lg,%s,%lg,%lg,%lu\n", 
                   NUM_ASSETS, NUM_RUNS, NUM_MONTHS, num_sim_threads,
                   get_from_yahoo, retrieval_time, sim_time, PORT_SUFF,
                   res_mean, res_std, init_seed);
        } else {
            printf("ERROR: File for test results could not be opened\n");
            exit(1);
        }
        fclose(test_file);
    } else {
        printf("NO RESULTS FILE\n");
    }
    gsl_matrix_free(cholesky);
    exit(0);
}
