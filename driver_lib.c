#include "driver_lib.h"
#include <gsl/gsl_linalg.h>

#ifndef GSL_MAT_H
#define GSL_MAT_H
#include <gsl/gsl_matrix.h>
#endif

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

    return varcovar;
}

struct risky_asset* assets_from_file(const char *filename,
        const int NUM_ASSETS) {

    struct risky_asset *assets = (struct risky_asset *)malloc(NUM_ASSETS *
            sizeof(struct risky_asset));

    FILE *asset_file = fopen("assets.csv", "r");
    if (asset_file) {
        for (int i = 0; i < NUM_ASSETS; i++) {
            fscanf(asset_file, "%lg,%lg,%lg", &assets[i].mean, &assets[i].sigma,
                    &assets[i].port_weight);
        }
    } else {
        printf("ERROR: valid asset file not provided\n");
        exit(1);
    }

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
