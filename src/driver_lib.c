#include "driver_lib.h"
#include <gsl/gsl_linalg.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>
#include <math.h>
#include <gsl/gsl_statistics_double.h>

#ifndef GSL_MAT_H
#define GSL_MAT_H
#include <gsl/gsl_matrix.h>
#endif

/* Must make sure that upr buffer is the same size as the src buffer */
char *str_upr(char *upr, char *src) {
    for (int i = 0; src[i]; i++) {
        upr[i] = toupper(src[i]);
    }
    return upr;
}

/* Don't forget to free the memory taken by the filename */
char *get_stock_file(char *ticker, struct tm start_time, int num_years) {

    char *tick_upr = malloc((strlen(ticker) + 1) * sizeof(char));
    str_upr(tick_upr, ticker);

    /* Build URL string for yahoo finance - URL usually contains ~ 100 chars */
    char url[200];
    sprintf(url, "http://real-chart.finance.yahoo.com/table.csv?s=%s&a=%02d&b=%02d&c=%04d&d=%02d&e=%02d&f=%04d&g=m&ignore=.csv",
            tick_upr, start_time.tm_mon, start_time.tm_mday,
            start_time.tm_year + 1900 - num_years, start_time.tm_mon,
            start_time.tm_mday, start_time.tm_year + 1900);

    /* Create the file name string, simply the stock ticker with an extension */
    char ext[] = "csv";
    char subdir[] = "data";

    /* Add 3 for the '/' '.' and null character */
    char *filename = malloc((strlen(tick_upr) + strlen(ext) + 3) * sizeof(char));
    sprintf(filename, "%s/%s.%s", subdir, tick_upr, ext);
    FILE *file = fopen(filename, "w");

    curl_global_init(CURL_GLOBAL_ALL);
    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
    CURLcode res = curl_easy_perform(handle);
    curl_global_cleanup();
    fclose(file);

    return filename;
}


/* Sets double array with the monthly returns for the stock whose
 * data is contained in the given file */
double *read_price_file(char *filename, size_t *data_size) {

    FILE *data_file = fopen(filename, "r");

    /* Monthly data for the past five years for each stock will most likely
     * be the number of data points used in each stock return CSV file */
    size_t ret_cap = 65;
    size_t num_rets = 0;
    double *ret_data = malloc(ret_cap * sizeof(double));

    if (data_file) {
        double prev_price = -1, curr_price = -1;

        /* Throw away the first line of input */
        int scanned = fscanf(data_file, "%*s");
        
        while (scanned != EOF) {
            /* Make sure that array is large enough to store next return
             * data point */
            if (num_rets >= ret_cap) {
                /* Double the size of the array */
                ret_cap *= 2;
                double *tmp = realloc(ret_data, (ret_cap + 1) * sizeof(double));
                /*+1 ensures that if ret_cap is 0, some data will be allocated*/
                if (!tmp) {
                    printf("ERROR: Array could not be expanded\n");
                    exit(1);
                }
                ret_data = tmp;
            }


            /* If this is the first line to be read, 
             * there is no current price, so read in first price */
            if (curr_price < 0) {
                scanned = fscanf(data_file, "%*[^,],%*g,%*g,%*g,%*g,%*d,%lg", &curr_price);
            } else {
                scanned = fscanf(data_file, "%*[^,],%*lg,%*lg,%*lg,%*lg,%*d,%lg", &prev_price);
                /* Formula for return = ln(priceCurr/pricePrev) */
                if (scanned != EOF) {
                    ret_data[num_rets] = log(curr_price / prev_price);
                    num_rets++;
                    /* Parallelism inhibitor: loop-carried dependency */
                    curr_price = prev_price;
                }
            }
        }
    } else {
        printf("ERROR: Could not open file\n");
        exit(1);
    }

    *data_size = num_rets;
    return ret_data;
}

/* Returns an array of strings containing the stock tickers for each stock */
char **read_ticker_file(char *filename, const size_t *NUM_STOCKS) {
    FILE *ticker_file = fopen(filename, "r");
    char **tickers;
    size_t MAX_TICK_SIZE = 5;
    if (ticker_file) {
        char line[MAX_TICK_SIZE + 1];
        fgets(line, sizeof line, ticker_file);
        int scanned = sscanf(line, "%d", NUM_STOCKS);
        tickers = malloc(*NUM_STOCKS * sizeof(char *));
        for (int i = 0; i < *NUM_STOCKS && scanned != EOF; i++) {
            char *temp = malloc((MAX_TICK_SIZE + 1) * sizeof(char));
            fgets(line, sizeof line, ticker_file);
            scanned = sscanf(line, "%5s", temp);
            tickers[i] = temp;
        }
    } else {
        printf("ERROR: Ticker file could not be opened\n");
        exit(1);
    }
    fclose(ticker_file);

    
    return tickers;
}

double *read_weight_file(char *filename, const size_t NUM_STOCKS) {
    FILE *file = fopen(filename, "r");
    double *weights = malloc(NUM_STOCKS * sizeof(double));
    if (file) {
        int scanned = 0;
        double sum = 0;
        for (int i = 0; i < NUM_STOCKS && scanned != EOF; i++)
            scanned = fscanf(file, "%lg", &weights[i]);
        for (int i = 0; i < NUM_STOCKS; i++)
            sum += weights[i];
        if (sum != 1) {
            printf("ERROR: Weights do not add up to 100%\n");
            exit(1);
        }
    } else {
        printf("ERROR: Weights file could not be opened\n");
        exit(1);
    }
    return weights;
}

gsl_matrix *calculate_varcovar(ret_data *dataset, size_t NUM_STOCKS) {

    /* Create the varcorvar matrix */
    gsl_matrix *varcovar = gsl_matrix_alloc(NUM_STOCKS, NUM_STOCKS);

    /* Fill the upper triangle (and diagonal) of the varcovar matrix 
     * by calculating the covariance */
    for (int i = 0; i < NUM_STOCKS; i++) {
        for (int j = i; j < NUM_STOCKS; j++) {
            /* Assumes that datasets will have the same size - a reasonable
             * assumption since all stocks will have return data going back
             * to and from the same points in time */
            gsl_matrix_set(varcovar, i, j,
                gsl_stats_covariance(dataset[i].data, 1,
                    dataset[j].data, 1, dataset[i].size));
        }
    }

    /* Fill lower triangle of the varcovar matrix */
    for (int i = 1; i < NUM_STOCKS; i++) {
        for (int j = 0; j < i; j++) {
            gsl_matrix_set(varcovar, i, j,
                    gsl_matrix_get(varcovar, j, i));
        }
    }

    return varcovar;

}

gsl_matrix* varcovar_from_file(const char *filename, const int *NUM_ASSETS) {
    gsl_matrix *varcovar;
    FILE *vc_file = fopen(filename, "r");
    if (vc_file) {

        /* Get the number of assets in the portfolio as first line in file */
        fscanf(vc_file, "%d", NUM_ASSETS);

        /* Create matrix with size NUM_ASSETS by NUM_ASSETS */
        varcovar = gsl_matrix_alloc(*NUM_ASSETS, *NUM_ASSETS);

        /* Read all numbers from file and place into matrix */
        for (int i = 0; i < *NUM_ASSETS; i++) {
            for (int j = 0; j < *NUM_ASSETS; j++) {
                double curr; 
                fscanf(vc_file, "%lf,", &curr);
                gsl_matrix_set(varcovar, i, j, curr);
            } 
        }
    } else {
        printf("ERROR: no variance-covariance matrix file provided\n");
        exit(1);
    }
    fclose(vc_file);

    return varcovar;
}

risky_asset* assets_from_file(const char *filename,
        const int NUM_ASSETS) {

    risky_asset *assets = (risky_asset *)malloc(NUM_ASSETS *
            sizeof(risky_asset));

    FILE *asset_file = fopen(filename, "r");
    if (asset_file) {
        for (int i = 0; i < NUM_ASSETS; i++) {
            fscanf(asset_file, "%lg,%lg,%lg", &assets[i].mean, &assets[i].sigma,
                    &assets[i].port_weight);
        }
    } else {
        printf("ERROR: valid asset file not provided\n");
        exit(1);
    }
    fclose(asset_file);

    double total_weight = 0;
    for (int i = 0; i < NUM_ASSETS; i++) {
        total_weight += assets[i].port_weight;
    }
    if (total_weight != 1.0) {
        printf("ERROR: weights of assets do not total 100%%\n");
        exit(1);
    }

    return assets;
}

void perform_cholesky(gsl_matrix *matrix, const int NUM_ASSETS) {

    gsl_linalg_cholesky_decomp(matrix);

    /* Make the Cholesky decomposition matrix L lower triangular */
    for (int i = 0; i < NUM_ASSETS; i++) {
        for (int j = i+1; j < NUM_ASSETS; j++) {
            gsl_matrix_set(matrix,i,j,0.0);
        }
    }

}
