#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>  
#include <stdexcept>
#include <string>

#include "mpi.h"

using namespace std;

int main(int argc, char *argv[])
{

int i, n;
int rank,size;

MPI_Init(&argc, &argv);

MPI_Comm_rank(MPI_COMM_WORLD, &rank);

MPI_Comm_size(MPI_COMM_WORLD, &size);

if(size < 2) {
  cout << "Not enough ranks to ping pong please sue at least 2" << endl;
}

int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    int num_sizes = sizeof(sizes) / sizeof(int);
    int iterations = 10000;
    double start_time, end_time, elapsed_time;

    for (int i = 0; i < num_sizes; i++) {
        int msg_size = sizes[i];
        char *buffer = (char *)malloc(msg_size * sizeof(char));

        if (rank == 0) {
            printf("Testing message size: %d bytes\n", msg_size);
        }

        MPI_Barrier(MPI_COMM_WORLD); // Make sure both processes start at roughly the same time
        start_time = MPI_Wtime();

        for (int j = 0; j < iterations; j++) {
            if (rank == 0) {
                MPI_Send(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else if (rank == 1) {
                MPI_Recv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        end_time = MPI_Wtime();
        elapsed_time = end_time - start_time;

        if (rank == 0) {
            printf("Time for %d iterations with message size %d bytes: %f seconds\n", iterations, msg_size, elapsed_time);
            // Output results to a CSV file with header
            ofstream outFile("results.csv", ios::app);  // Open file in append mode
            if (outFile.is_open()) {
                // Add header if the file is empty
                if (outFile.tellp() == 0) {
                    outFile << "bytes, wall_time ,iterations" << endl;
                }
                outFile << msg_size << "," << elapsed_time << "," << iterations << endl;
                outFile.close();
                cout << "Results appended to results.csv" << endl;
            } else {
                cerr << "Unable to open the output file." << endl;
                return 1;
            }
        }

        free(buffer);
        MPI_Barrier(MPI_COMM_WORLD); // Ensure all processes have finished before moving to the next message size
    }
MPI_Finalize();


}