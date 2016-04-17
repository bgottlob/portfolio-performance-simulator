#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_statistics_double.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <curl/curl.h>

struct risky_asset {
    double mean;
    double sigma;
    double port_weight;
};

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

struct risky_asset read_price_file(char *filename) {

    FILE *data_file = fopen(filename, "r");
    struct risky_asset stock;

    /* Monthly data for the past five years for each stock will most likely
     * be the number of data points used in each stock return CSV file */
    size_t ret_cap = 65;
    double *ret_data = (double *)malloc(ret_cap * sizeof(double));
    int num_rets = 0;

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
                double *tmp = realloc(ret_data, ret_cap * sizeof(double));
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

    /*for (int i = 0; i < num_rets; i++) {
        printf("%d: %lg\n", i, ret_data[i]);
    }*/

    stock.mean = gsl_stats_mean(ret_data, 1, num_rets);
    stock.sigma = gsl_stats_sd(ret_data, 1, num_rets);

    free(ret_data);

    return stock;

}

int main() {
    time_t t = time(NULL);
    struct tm curr_time = *localtime(&t);   
    char *tick = "aapl";
    struct risky_asset stock = read_price_file(get_stock_file(tick, curr_time, 6));
    printf("Info for %s\nMean: %lg\n Std: %lg\n", tick, stock.mean, stock.sigma);
    return 0;
}


