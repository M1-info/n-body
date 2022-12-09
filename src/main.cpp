#include "main.h"

int main(int argc, char *argv[])
{

    // init MPI
    MPI_Init(&argc, &argv);

    // get number of tasks
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // get current machine rank
    int current_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &current_rank);

    // get current processor name
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_length;
    MPI_Get_processor_name(hostname, &name_length);

    double *velocities = nullptr;
    velocities = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *local_forces = nullptr;
    local_forces = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *local_positions = nullptr;
    local_positions = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *tmp_forces = nullptr;
    tmp_forces = (double *)malloc(nb_body * sizeof(double));

    double *tmp_positions = nullptr;
    tmp_positions = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    MPI_Finalize();
    return 0;
}