#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mpi.h"


#define LIMIT 63488
#define ITERATIONS 1000
#define STEP 1024


int main(){
    int rank, size;
    char buf[LIMIT];
    int seed;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size < 3){
        printf("Less than 3 processes will no go");
        return -1;
    }
    int arr[size];
    int x;    

    if(rank == 0)
        seed = 10;
    MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    srand(seed + rank);

    if(rank == 1){
        printf("Arr: [");
        for(int i = 0; i < size; ++i){
            arr[i] = i;
            printf("%d, ", arr[i]);
        }
        printf("]\n");
    }
    MPI_Scatter(arr, 1, MPI_INT, &x, 1, MPI_INT, 1, MPI_COMM_WORLD);
    x += rand() % size;

    MPI_Gather(&x, 1, MPI_INT, arr, 1, MPI_INT, 2, MPI_COMM_WORLD);
    if(rank == 2){
        printf("Arr: [");
        for(int i = 0; i < size; ++i)
            printf("%d, ", arr[i]);
        printf("]\n");
    }

    MPI_Finalize();
    return 0;
}
