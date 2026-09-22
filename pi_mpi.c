#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
int main(int argc, char **argv)
{
int rank, size;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
long long n = 10000000;
int valid = 1;
if (rank == 0) {
if (argc > 2)
valid = 0;
if (argc == 2) {
char *end = NULL;
errno = 0;
n = strtoll(argv[1], &end, 10);
if (errno != 0 || end == argv[1] || *end != '\0'
|| n < 1 || n > 1000000000LL)
valid = 0;
}
}
MPI_Bcast(&valid, 1, MPI_INT, 0, MPI_COMM_WORLD);
if (!valid) {
if (rank == 0)
fprintf(stderr,
"Usage: ./pi_mpi [N: 1..1000000000]\n");

MPI_Finalize();
return 1;
}
MPI_Bcast(&n, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
long long q = n / size, rem = n % size;
long long count = q + (rank < rem ? 1 : 0);
long long start = rank * q + (rank < rem ? rank : rem);
long long end = start + count;
double h = 1.0 / (double)n;
MPI_Barrier(MPI_COMM_WORLD);
double t0 = MPI_Wtime();
double local_sum = 0.0;
for (long long i = start; i < end; ++i) {
double x = ((double)i + 0.5) * h;
local_sum += 4.0 / (1.0 + x * x);
}
double local_pi = h * local_sum;
double pi = 0.0;
MPI_Reduce(&local_pi, &pi, 1, MPI_DOUBLE,
MPI_SUM, 0, MPI_COMM_WORLD);
double elapsed = MPI_Wtime() - t0;
double max_elapsed = 0.0;
MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE,

MPI_MAX, 0, MPI_COMM_WORLD);

if (rank == 0) {
double reference = acos(-1.0);
printf("N=%lld processes=%d pi=%.15f\n",
n, size, pi);
printf("absolute_error=%.3e time_seconds=%.6f\n",
fabs(pi - reference), max_elapsed);

}
MPI_Finalize();
return 0;
}
