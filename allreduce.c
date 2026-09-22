#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
double local = rank + 1.0;
double global_sum = 0.0;
MPI_Allreduce(&local, &global_sum, 1, MPI_DOUBLE,

MPI_SUM, MPI_COMM_WORLD);
double mean = global_sum / size;
printf("rank=%d sum=%.1f mean=%.1f\n",
rank, global_sum, mean);
MPI_Finalize();
return 0;
}
