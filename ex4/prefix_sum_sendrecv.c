// 330829847 Ido Maimon
// 216764803 Yuli Smishkis

#include <mpi.h>
#include <stdio.h>

/*
 * Parallel inclusive prefix sum across MPI ranks.
 * Each rank starts with x_r = r and computes y_r = sum_{k=0..r} x_k
 * using O(log P) rounds of point-to-point communication (Hillis-Steele).
 *
 * Uses only MPI_Send and MPI_Recv (no MPI_Scan, no local 0..r loop).
 * Ordering by rank parity keeps blocking Send/Recv deadlock-free;
 * MPI_PROC_NULL turns out-of-range partners into no-ops.
 */
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sum = rank;

    for (int step = 1; step < size; step *= 2) {
        const int send_to   = (rank + step < size) ? (rank + step) : MPI_PROC_NULL;
        const int recv_from = (rank - step >= 0)   ? (rank - step) : MPI_PROC_NULL;

        const int send_val = sum;   /* value at the start of this round */
        int recv_val = 0;

        if (rank % 2 == 0) {
            MPI_Send(&send_val, 1, MPI_INT, send_to,   0, MPI_COMM_WORLD);
            MPI_Recv(&recv_val, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            MPI_Recv(&recv_val, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&send_val, 1, MPI_INT, send_to,   0, MPI_COMM_WORLD);
        }

        if (recv_from != MPI_PROC_NULL) {
            sum += recv_val;
        }
    }

    printf("rank=%d x=%d prefix=%d\n", rank, rank, sum);

    MPI_Finalize();
    return 0;
}