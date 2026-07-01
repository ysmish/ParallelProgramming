// 330829847 - Ido Maimon
// 216764803 - Yuli Smishkis

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

// Calculates the number of rows each process receives
int get_rows_for_rank(int r, int N, int P);

// Calculates the matrix multiplication
int* matrix_calc(const int* A, const int* B, int rowA, int colA, int rowB, int colB);

/*
 * This main function calculates a parallel matrix multiplication using 1D Row-Block Distribution
 */
int main(int argc, char **argv) {

    // 4 arguments + program name
    if (argc != 5) {
        return 1;
    }

    // Initializing the environment
    MPI_Init(&argc, &argv);

    /*
     * rank - the id of the process
     * size - number of processes
     */
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Extracting the command line arguments
    const char* N_char = argv[1];
    const char* seedA_char = argv[2];
    const char* seedB_char = argv[3];
    const char* maxValue_char = argv[4];

    // Casting to integers
    const int N = atoi(N_char);
    const int seedA = atoi(seedA_char);
    const int seedB = atoi(seedB_char);
    const int maxValue = atoi(maxValue_char);

    // Allocating the memory for the matrix B
    IMatrix B = imatrix_alloc(N);

    // Declaring the sendBuffer pointer. for rank = 0 it will be A.data and for everyone else it is NULL
    int* sendBuffer = NULL;

    // Filling the matrices with random values - only for rank = 0
    if (rank == 0) {

        // Only rank 0 has the entire matrix
        IMatrix A = imatrix_alloc(N);
        sendBuffer = A.data;

        // Randomizing
        imatrix_fill_random(&A, seedA, maxValue);
        imatrix_fill_random(&B, seedB, maxValue);
    }

    /*
     * Broadcasting B matrix to everyone.
     * Since Imatrix is not a valid MPI type - we only pass the values in 'data'
     */
    MPI_Bcast(B.data, N * N, MPI_INT, 0, MPI_COMM_WORLD);


    // Parameters for the scatterv function that is used to send the different rows of A

    // Allocating the sendcounts and displs arrays
    int* sendCounts = calloc(size, sizeof(int));
    int* displs = calloc(size, sizeof(int));

    /*
     * Process 0 calculates the number of elements to send to each process using the provided formula.
     * Also calculates the first row each process gets
     */
    if (rank == 0) {
        for (int i = 0; i < size; i++) {

            // Using the provided formula to calculate the first row for each process
            const int first_row = i * N / size;

            // The first integer in the row is at index row_number * N
            displs[i] = N * first_row;

            // Calculating the number of rows each process gets
            sendCounts[i] = get_rows_for_rank(i, N, size) * N;
        }
    }

    // Now calculating locally for each process the number of integers he receives so it can allocate its memory
    const int number_of_integers = get_rows_for_rank(rank, N, size) * N;
    const int row_count = get_rows_for_rank(rank, N, size);

    // Allocate the receiveBuffer
    int* receiveBuffer = malloc(number_of_integers * sizeof(int));

    // Sending the data
    MPI_Scatterv(sendBuffer, sendCounts, displs, MPI_INT, receiveBuffer, number_of_integers,
        MPI_INT, 0, MPI_COMM_WORLD);

    // Calculating the multiplication
    int* localC = matrix_calc(receiveBuffer, B.data, row_count, N, N, N);

    /*
     * Now we need to send to the root the matrix. Just like in the scatterv, we declare a receiveBuffer (the C matrix)
     * which is NULL for all other processes
     */
    int* resultBuffer = NULL;

    // Allocate the memory for the resultBuffer for the root
    if (rank == 0) {
        resultBuffer = malloc(N * N * sizeof(int));
    }

    // Sending the data
    MPI_Gatherv(localC, number_of_integers, MPI_INT, resultBuffer, sendCounts, displs,
        MPI_INT, 0, MPI_COMM_WORLD);

    // The root process now stores the data in an IMatrix struct
    if (rank == 0) {

        // Storing the values in the C matrix
        IMatrix C;
        C.N = N;
        C.data = resultBuffer;

        // Printing the sum of C matrix as required
        printf("checksum(C)=%lld\n", imatrix_checksum(&C));
    }


    // Freeing the memory
    imatrix_free(&B);

    free(sendCounts);
    free(displs);
    free(sendBuffer);
    free(receiveBuffer);
    free(localC);
    free(resultBuffer);

    // Closing the MPI
    MPI_Finalize();
    return 0;
}

// r - rank, N - dimension of the space of the matrix, P - number of processes
int get_rows_for_rank(const int r, const int N, const int P) {
    const int first_row = r * N / P;
    const int last_row = (r + 1) * N / P - 1;
    const int row_count = last_row - first_row + 1;
    return row_count;
}

// Calculating A * B and returning C - the result matrix
int* matrix_calc(const int* A, const int* B, const int rowA, const int colA, const int rowB, const int colB) {

    // The matrices cannot be multiplied
    if (colA != rowB) {
        return NULL;
    }

    // Allocating the memory for the result matrix. the size is rowA * colB
    int* C = calloc(rowA * colB, sizeof(int));

    for (int i = 0; i < rowA; i++) {
        for (int j = 0; j < colB; j++) {

            // for C_[i,j] we need to calculate the scalar product of row i in A and column j in B
            for (int k = 0; k < colA; k++) {
                C[i * colB + j] += A[i * colA + k] * B[k * colB + j];
            }
        }
    }

    return C;
}