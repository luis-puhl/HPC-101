#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[], char *env[]) {
    MPI_Init(&argc, &argv);
    int mpi_rank, mpi_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // setup
    size_t size = 3;
    double a[size*size], b[size*size], c[size*size];
    if (mpi_rank == 0) {
        // int midway = size / 2;
        for (size_t i = 0; i < size; i++) {
            // printf("| ");
            for (size_t j = 0; j < size; j++) {
                a[i * size + j] = i == j;
                b[i * size + j] = 2.0;
                // c[i * size + j] = 0.0;
                // printf("%0.2lf ", a[i * l_size + j]);
            }
            // printf("| %c |", i == midway ? 'x' : ' ');
            // for (size_t j = 0; j < l_size; j++) {
            //     printf("%0.2lf ", b[i * l_size + j]);
            // }
            // printf("| \n");
        }
    }

    struct timeval time_start, time_end;
    gettimeofday(&time_start, NULL);

    // work
    int chunk = size / mpi_size;
    int remainder = size % mpi_size;
    int begin = mpi_rank * chunk;
    int end = begin + chunk + (mpi_rank < remainder);
    if (mpi_rank == 0) {
        for (int i = 1; i < mpi_size; i++) {
            int begin_i = i * chunk;
            int end_i = begin_i + chunk + (i < remainder);
            // printf("send [%d] %d -> %d\n", i, begin_i, end_i);
            // MPI_Send(&a[begin_i], end_i - begin_i, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(a, size * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(b, size * size, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        }
    } else {
        // MPI_Recv(&a[begin], end - begin, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, NULL);
        MPI_Recv(a, size * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, NULL);
        MPI_Recv(b, size * size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, NULL);
    }
    printf("[%d] %d -> %d\n", mpi_rank, begin, end);
    for (size_t i = begin; i < end; i++) {
        for (size_t j = 0; j < size; j++) {
            c[i * size + j] = 0.0;
            for (size_t k = 0; k < size; k++){
                c[i * size + j] += a[i * size + k] * b[k * size + j];
            }
            printf("[%d] c(%ld, %ld)=%lf\n", mpi_rank, i, j, c[i * size + j]);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank == 0) {
        for (size_t i = 1; i < mpi_size; i++) {
            int begin_i = i * chunk;
            int end_i = begin_i + chunk + (i < remainder);
            MPI_Recv(&c[begin_i], end_i - begin_i, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, NULL);
        }
    } else {
        MPI_Send(&c[begin], end - begin, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
    }

    gettimeofday(&time_end, NULL);
    double exec_time =
        (double)(time_end.tv_sec - time_start.tv_sec) +
        (double)(time_end.tv_usec - time_start.tv_usec) / 1000000.0;
    int midway = size / 2;
    char printout[size * size * 3 * 8];
    size_t n = sprintf(printout, "rank %d\n", mpi_rank);
    for (size_t i = 0; i < size; i++) {
        n += sprintf(&printout[n], "| ");
        for (size_t j = 0; j < size; j++) {
            n += sprintf(&printout[n], " % 0.2lf", a[i * size + j]);
        }
        n += sprintf(&printout[n], " | %c |", i == midway ? 'x' : ' ');
        for (size_t j = 0; j < size; j++) {
            n += sprintf(&printout[n], " % 0.2lf", b[i * size + j]);
        }
        n += sprintf(&printout[n], " | %c |", i == midway ? '=' : ' ');
        for (size_t j = 0; j < size; j++) {
            n += sprintf(&printout[n], "% 0.2lf ", c[i * size + j]);
        }
        n += sprintf(&printout[n], "| \n");
    }
    fprintf(stderr, "%s", printout);
    if (mpi_rank == 0) {
        printf("%s in %lf seconds with n=%d\n", argv[0], exec_time, mpi_size);
    }

    return MPI_Finalize();
}
