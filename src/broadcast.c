#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
int iterations = 0;
if (rank == 0)
iterations = 1000;
MPI_Bcast(&iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);
printf("rank=%d iterations=%d\n", rank, iterations);
MPI_Finalize();
return 0;
}
