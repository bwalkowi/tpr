#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"


#define RAND ((long double)rand() / (long double)RAND_MAX)
#define ITERATIONS 1000000


int pi(int iterations){
    int in_circle = 0;
    long double x, y;
    for(int i = 0; i < iterations; ++i){
        x = RAND;
        y = RAND;
        if((x*x + y*y) <= 1)
            ++in_circle;
    }
    return in_circle;
}


int main(){
    int rank, size;
    long double t1, t2;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

    MPI_Barrier(MPI_COMM_WORLD);
    t1 = MPI_Wtime();

    int in_circle = pi(ITERATIONS);
    int all_in_circle = 0;
    int all_points = size*ITERATIONS;

    MPI_Reduce(&in_circle, &all_in_circle, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    t2 = MPI_Wtime();
    
    if(rank == 0){
        long double po_2_pd = ((long double)all_in_circle)/((long double)all_points);
        printf("\nTIME: %.10Lf\nPI: %.10Lf\n", t2-t1, 4.0*po_2_pd);
    }

    MPI_Finalize();
    return 0;
}
