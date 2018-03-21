#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"


#define RAND ((long double)rand() / (long double)RAND_MAX)


long long monte_carlo(long long iterations){
    long long in_circle = 0;
    long double x, y;
    for(long i = 0; i < iterations; ++i){
        x = RAND;
        y = RAND;
        if((x*x + y*y) <= 1)
            ++in_circle;
    }
    return in_circle;
}


int main(int argc, char *argv[]){
    int rank, size;
    long double t1, t2;

    int exponent = atoi(argv[1]);
    long long iterations = 10;
    for(int i = 1; i < exponent; ++i)
        iterations *= 10;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(rank);
    long long in_circle_global;
    long long points_overall = size*iterations;

    MPI_Barrier(MPI_COMM_WORLD);

    t1 = MPI_Wtime();
    long long in_circle_local = monte_carlo(iterations);
    MPI_Reduce(&in_circle_local, &in_circle_global, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    long double pi = 4.0*(long double)in_circle_global/(long double)points_overall;
    t2 = MPI_Wtime();
    
    if(rank == 0)
        printf("%.10Lf %.10Lf\n", t2-t1, pi);

    MPI_Finalize();
    return 0;
}
