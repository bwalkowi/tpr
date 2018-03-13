#include <stdio.h>
#include "mpi.h"


#if defined SYNC
    #define SEND MPI_Ssend
#else
    #define SEND MPI_Send
#endif


#define ITERATIONS 1000000


int main(){
    int rank;
    int msg = 8008;
    long double t1, t2;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // assure processes starts pingpong at the same time
    MPI_Barrier(MPI_COMM_WORLD);

    t1 = MPI_Wtime();
    for(int i = 0; i < ITERATIONS; ++i){
        if(rank == 0){
            SEND(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else{
            MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            SEND(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    t2 = MPI_Wtime();

    /* 
     * 2*latency = RTT (round trip time) -> 1 pingpong
     * 2*n*latency = n*RTT               -> for averaging
     * latency = n*RTT / 2*n = (t2-t1) / (2*n)
     */
    if(rank == 0)
        printf("%.10Lf ms\n", 1000.0 * (t2 - t1) / (2.0 * ITERATIONS));

    MPI_Finalize();
    return 0;
}
