#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
int rank, size, name_len;
char name[MPI_MAX_PROCESSOR_NAME];
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
MPI_Get_processor_name(name, &name_len);
int x = 0;
if (rank == 0)
x = 100;
printf("rank=%d size=%d host=%s x=%d\n",
rank, size, name, x);
MPI_Finalize();
return 0;
}
