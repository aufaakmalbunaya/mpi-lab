#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
int left = (rank - 1 + size) % size;
int right = (rank + 1) % size;
int outgoing = rank, incoming = -1;
MPI_Request requests[2];
MPI_Irecv(&incoming, 1, MPI_INT, left, 0,
MPI_COMM_WORLD, &requests[0]);
MPI_Isend(&outgoing, 1, MPI_INT, right, 0,
MPI_COMM_WORLD, &requests[1]);

/* Independent work: neither buffer is accessed. */
double work = 0.0;
for (int i = 1; i <= 10000; ++i)
work += 1.0 / (i + rank + 1.0);
MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
printf("rank=%d received=%d work=%.6f\n",
rank, incoming, work);
MPI_Finalize();
return 0;
}
