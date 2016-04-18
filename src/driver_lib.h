#ifndef GSL_MAT_H
#define GSL_MAT_H
#include <gsl/gsl_matrix.h>
#endif

#ifndef DRIVER_DEFS_H
#define DRIVER_DEFS_H

#include "portfolio_lib.h"

/* Initialize a variance covariance matrix based on the contents of a file
 * Allocates memory for the matrix - make sure to free the memory at the
 * address of the returned pointer later. Will modify NUM_ASSETS */
gsl_matrix* varcovar_from_file(const char *filename, const int *NUM_ASSETS);

/* Create an array of risky_asset structs from the contents of a file given
 * the number of assets to be created */
struct risky_asset* assets_from_file(const char *filename,
        const int NUM_ASSETS);

void perform_cholesky(gsl_matrix *matrix, const int NUM_ASSETS);

#endif