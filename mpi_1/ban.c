#include <stdio.h>
#include "mpi.h"


#if defined SYNC
    #define SEND MPI_Ssend
#else
    #define SEND MPI_Send
#endif


#define LIMIT 64000
#define ITERATIONS 1000000
#define STEP 1000


int main(){
    int rank;
    char buf[LIMIT];

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // assure processes starts pingpong at the same time
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0){
        long double t1, t2;
        long double avg_time;
        for(int size = 0; size <= LIMIT; size += STEP){
            t1 = MPI_Wtime();
            for(int i = 0; i < ITERATIONS; ++i)
                SEND(buf, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
            t2 = MPI_Wtime();
            avg_time = (t2 - t1) / ITERATIONS;
            printf("%d;%.10Lf;%.10Lf\n", size, avg_time, size / avg_time);
        }
    }
    else{
        for(int size = 0; size <= LIMIT; size += STEP)
            for(int i = 0; i < ITERATIONS; ++i)
                MPI_Recv(buf, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Finalize();
    return 0;
}
