#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
int right = (rank + 1) % size;
int left = (rank - 1 + size) % size;
int outgoing = rank;
int incoming = -1;
MPI_Sendrecv(&outgoing, 1, MPI_INT, right, 0,
&incoming, 1, MPI_INT, left, 0,
MPI_COMM_WORLD, MPI_STATUS_IGNORE);
printf("rank=%d sent=%d to=%d received=%d from=%d\n",
rank, outgoing, right, incoming, left);
MPI_Finalize();
return 0;
}
