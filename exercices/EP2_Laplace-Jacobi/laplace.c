#include "laplace.h"

#include <stdio.h>
#include <stdlib.h>

Grid laplace_init(size_t size) {
    // allocate memory to the grid (matrix)
    // matrix to be solved
    Grid grid;
    grid.size = size;
    grid.now = (double **)calloc(size, sizeof(double *));
    grid.next = (double **)calloc(size, sizeof(double *));

    int inferior = size / 2;
    int superior = inferior + size / 10;
    superior = superior == inferior ? superior + 1 : superior;
    // fprintf(stderr, "inferior=%d, superior=%d\n", inferior, superior);
    // set grid initial conditions
    for(int i = 0; i < size; i++){
        grid.now[i] = (double *) calloc(size, sizeof(double));
        grid.next[i] = (double *) calloc(size, sizeof(double));
        for (int j = 0; j < size; j++){
            // inicializa região de calor no centro do grid
            if ( i>=inferior && i < superior && j>=inferior && j<superior) {
                grid.now[i][j] = 100;
            } else {
                grid.now[i][j] = 0;
            }
            // new_grid[i][j] = 0.0;
            // grid[i][j] = rand() % 100;
            grid.next[i][j] = grid.now[i][j];
            printf("%lf ", grid.now[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    return grid;
}

int laplace_print(Grid *grid) {
    int n = 0;
    for(int i = 0; i < grid->size; i++){
        for(int j = 0; j < grid->size; j++){
            n += printf("%lf ", grid->now[i][j]);
        }
        n += printf("\n");
    }
    n += printf("\n");
    return n;
}

double laplace_iterate_cell(Grid *grid, size_t i, size_t j) {
    if (i == 0 || j == 0 || i >= (grid->size - 1) || j >= (grid->size - 1)) {
        return 0.0;
    }
    grid->next[i][j] = 0.25 * (grid->now[i][j+1] + grid->now[i][j-1] +
                                         grid->now[i-1][j] + grid->now[i+1][j]);
    // err = max(err, absolute(new_grid[i][j] - grid[i][j]));
    double diff = grid->next[i][j] - grid->now[i][j];
    if (diff < 0) {
        diff = (-diff);
    }
    // fprintf(stderr, "diff=%le\n", diff);
    return diff;
}
