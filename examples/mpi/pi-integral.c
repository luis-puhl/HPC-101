#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

int main(int argc, char *argv[], char *env[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    int num_steps = (int) 1e9;
    double step = 1.0 / (double) num_steps;
    int chunk = num_steps / size;
    int begin = rank * chunk;
    int end = (rank +1 != size) ? (begin + chunk) : (num_steps);
    
    double x, sum = 0.0;
    for (int i = begin + 1; i <= end; i++){
        x = (i - 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    double pi = 0.0;
    MPI_Reduce(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    pi = step * pi;

    // get the end time
    gettimeofday(&time_end, NULL);

    double exec_time = (double) (time_end.tv_sec - time_start.tv_sec) +
                       (double) (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    if (rank == 0) {
        printf("pi with %d steps is %.9lf in %lf seconds\n", num_steps, pi, exec_time);
    }

    return MPI_Finalize();
}
