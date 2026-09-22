#include <mpi.h>
#include <stdio.h>
int main(int argc, char **argv)
{
enum { N = 12 };
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
if (size > N || N % size != 0) {
if (rank == 0)
fprintf(stderr, "Use 1, 2, 3, 4, 6, or 12 ranks.\n");
MPI_Finalize();
return 1;
}
/* Only rank 0 initializes the full input. */
int data[N] = {0};
if (rank == 0)
for (int i = 0; i < N; ++i)
data[i] = i + 1;
int count = N / size;
int local[N] = {0};
MPI_Scatter(data, count, MPI_INT,

local, count, MPI_INT, 0, MPI_COMM_WORLD);

int local_sum = 0;
for (int i = 0; i < count; ++i)
local_sum += local[i];
int partials[N] = {0};
MPI_Gather(&local_sum, 1, MPI_INT,

partials, 1, MPI_INT, 0, MPI_COMM_WORLD);

int total = 0;
MPI_Reduce(&local_sum, &total, 1, MPI_INT,
MPI_SUM, 0, MPI_COMM_WORLD);

if (rank == 0) {
printf("Gathered partial sums:");
for (int r = 0; r < size; ++r)
printf(" %d", partials[r]);
printf("\nReduced total: %d\n", total);
}
MPI_Finalize();
return 0;
}
