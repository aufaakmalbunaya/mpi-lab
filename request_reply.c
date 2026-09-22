#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
if (size != 2) {
if (rank == 0)
fprintf(stderr, "Use exactly 2 processes.\n");
MPI_Finalize();
return 1;
}
if (rank == 0) {
int value = 21, reply = 0;
MPI_Send(&value, 1, MPI_INT, 1, 10,
MPI_COMM_WORLD);
MPI_Recv(&reply, 1, MPI_INT, 1, 20,
MPI_COMM_WORLD, MPI_STATUS_IGNORE);
printf("Rank 0 sent %d and received %d\n",
value, reply);
} else {
int value = 0;
MPI_Recv(&value, 1, MPI_INT, 0, 10,
MPI_COMM_WORLD, MPI_STATUS_IGNORE);
int reply = 2 * value;
MPI_Send(&reply, 1, MPI_INT, 0, 20,
MPI_COMM_WORLD);

}
MPI_Finalize();
return 0;
}
