#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>

#include "mpi.h"

using namespace std;

#define ROUNDS 100

int main(int argc, char *argv[]) {

    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size < 2) {
        cout << "Not enough ranks to ping pong please use at least 2" << endl;
        MPI_Abort(MPI_COMM_WORLD, 1); // Exit if not enough processes
    }

    int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int num_sizes = sizeof(sizes) / sizeof(int);
    int iterations = 10000;
    double start_time, end_time, elapsed_time;

    MPI_Request send_request, recv_request;
    MPI_Status status;

    for (int i = 0; i < num_sizes; i++) {
        int msg_size = sizes[i];
        char *buffer = (char *)malloc(msg_size * sizeof(char));

        if (rank == 0) {
            printf("Testing message size: %d bytes\n", msg_size);
        }

        MPI_Barrier(MPI_COMM_WORLD); // Synchronize before starting
        start_time = MPI_Wtime();

        for (int j = 0; j < iterations; j++) {
            if (rank == 0) {
                MPI_Isend(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &send_request);
                MPI_Irecv(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &recv_request);
                MPI_Wait(&send_request, &status);
                MPI_Wait(&recv_request, &status);
            } else if (rank == 1) {
                MPI_Irecv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &recv_request);
                MPI_Isend(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &send_request);
                MPI_Wait(&recv_request, &status);
                MPI_Wait(&send_request, &status);
            }
        }

        end_time = MPI_Wtime();
        elapsed_time = end_time - start_time;

        if (rank == 0) {
            printf("Non-blocking time for %d iterations with message size %d bytes: %f seconds\n", iterations, msg_size, elapsed_time);
            // Output results to a CSV file
            ofstream outFile("results_non_blocking.csv", ios::app);
            if (outFile.is_open()) {
                if (outFile.tellp() == 0) { // Check if file is empty to write header
                    outFile << "bytes, wall_time, iterations" << endl;
                }
                outFile << msg_size << "," << elapsed_time << "," << iterations << endl;
                outFile.close();
                cout << "Results appended to results_non_blocking.csv" << endl;
            } else {
                cerr << "Unable to open the output file." << endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }

        free(buffer);
        MPI_Barrier(MPI_COMM_WORLD); // Synchronize again before next message size
    }

    MPI_Finalize();
    return 0;
}
