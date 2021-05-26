#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char *argv[], char *env[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("size %d, rank %d\n", size, rank);
    
    if (size <= 1) {
        printf("Cluster is too small.\n");
        MPI_Finalize();
        return -1;
    }

    if (rank == 0) {
        char *msg = "Hello MPI!\n";
        MPI_Send(msg, strlen(msg), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        char *buffer = calloc(140, sizeof(char));
        MPI_Recv(buffer, 140, MPI_CHAR, size - 1, 0, MPI_COMM_WORLD, NULL);
        printf("[%d] %s", rank, buffer);
    } else {
        char *msg = calloc(140, sizeof(char));
        MPI_Recv(msg, 140, MPI_CHAR, rank -1, 0, MPI_COMM_WORLD, NULL);
        printf("[%d] %s", rank, msg);
        MPI_Send(msg, strlen(msg), MPI_CHAR, (rank+1) % size, 0, MPI_COMM_WORLD);
    }

    return MPI_Finalize();
}
