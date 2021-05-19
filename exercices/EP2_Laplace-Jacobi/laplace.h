#ifndef LAPLACE_H
#define LAPLACE_H

#include <stdlib.h>

#define ITER_MAX 50000         // number of maximum iterations
#define CONV_THRESHOLD 1.0e-5f // threshold of convergence

typedef struct {
    size_t size;
    double **now, **next;
} Grid;

Grid laplace_init(size_t size);
int laplace_print(Grid *grid);
double laplace_iterate_cell(Grid *grid, size_t i, size_t j);

#endif // !LAPLACE_H
