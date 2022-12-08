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

    // number of bodies for each process
    int nb_body;

    // number of bodies left (if NB_BODY_TOTAL % world_size != 0)
    int nb_body_left;

    // compute nb_body to work with for each process
    if (current_rank == HOST_RANK)
        nb_body = floor(NB_BODY_TOTAL / world_size);

    // broadcast nb_body to all processes
    MPI_Bcast(&nb_body, 1, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    // compute nb_body_left
    nb_body_left = NB_BODY_TOTAL - (nb_body * world_size);

    if (current_rank == HOST_RANK && world_size > 1)
        nb_body += nb_body_left;

    // buffer for recvcounts and displs (used in MPI_Allgatherv)
    int *recvcounts = nullptr;
    recvcounts = (int *)malloc(world_size * sizeof(int));

    int *displs = nullptr;
    displs = (int *)malloc(world_size * sizeof(int));

    if (current_rank == HOST_RANK)
    {
        recvcounts[0] = nb_body * SENDED_DATA_SIZE;
        displs[0] = 0;
        for (int i = 1; i < world_size; i++)
        {
            recvcounts[i] = (nb_body - nb_body_left) * SENDED_DATA_SIZE;
            displs[i] = (nb_body + ((nb_body - nb_body_left) * (i - 1))) * SENDED_DATA_SIZE;
        }
    }

    // broadcast recvcounts and displs to all processes
    MPI_Bcast(&recvcounts[0], world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&displs[0], world_size, MPI_INT, HOST_RANK, MPI_COMM_WORLD);

    // buffers for MPI communication
    int *ids = nullptr;
    ids = (int *)malloc(NB_BODY_TOTAL * sizeof(int));
    double *masses = nullptr;
    masses = (double *)malloc(NB_BODY_TOTAL * sizeof(double));

    if (current_rank == HOST_RANK)
    {
        for (int i = 0; i < NB_BODY_TOTAL; i++)
        {
            ids[i] = i;
            masses[i] = randMinmax(10e2, 10e5);
        }
    }

    // broadcast ids and masses to all processes
    MPI_Bcast(&ids[0], NB_BODY_TOTAL, MPI_INT, HOST_RANK, MPI_COMM_WORLD);
    MPI_Bcast(&masses[0], NB_BODY_TOTAL, MPI_DOUBLE, HOST_RANK, MPI_COMM_WORLD);

    // velocity is created on each process and store own bodies velocity
    double *velocities = nullptr;
    velocities = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));

    double *positions = nullptr;
    positions = (double *)malloc(SENDED_DATA_SIZE * nb_body * sizeof(double));
    double *received_positions = nullptr;
    received_positions = (double *)malloc(SENDED_DATA_SIZE * NB_BODY_TOTAL * sizeof(double));

    // fill positions buffer
    for (int i = 0; i < nb_body; i++)
    {
        velocities[i * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = 0.0;
        velocities[i * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = 0.0;
        positions[i * SENDED_DATA_SIZE + POSITION_X_INDEX] = randMinmax(0, 10);
        positions[i * SENDED_DATA_SIZE + POSITION_Y_INDEX] = randMinmax(0, 10);
    }

    // Start time for perf measurements
    double start_time = MPI_Wtime();

    /* Main loop */
    for (int i = 0; i < NB_ITERATIONS; i++)
    {

        MPI_Allgatherv(
            &positions[0],
            SENDED_DATA_SIZE * nb_body,
            MPI_DOUBLE,
            &received_positions[0],
            &recvcounts[0],
            &displs[0],
            MPI_DOUBLE,
            MPI_COMM_WORLD);

        /* For each positions body received (id, mass and position), compute forces applied on the body */
        for (int j = 0; j < nb_body; j++)
        {
            double forces_on_body[2] = {0, 0};

            int id_current = ids[current_rank * nb_body + j];         // get current body id
            double mass_current = masses[current_rank * nb_body + j]; // get current body mass

            for (int k = 0; k < SENDED_DATA_SIZE * NB_BODY_TOTAL; k += SENDED_DATA_SIZE)
            {
                // get corresponding id of the other body
                int id_other = ids[k / SENDED_DATA_SIZE];

                // check if body is not the same
                if (id_other == id_current)
                    continue;

                // get mass and position of the other body
                double mass_other = masses[k];
                double position_other[2] = {received_positions[k + POSITION_X_INDEX], received_positions[k + POSITION_Y_INDEX]};

                // compute forces applied on the body
                double position_current[2] = {positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX],
                                              positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX]};

                computeForces(position_current, position_other, mass_current, mass_other, &forces_on_body[0]);
            }

            forces_on_body[0] *= -GRAVITATIONAL_CONSTANT * mass_current;
            forces_on_body[1] *= -GRAVITATIONAL_CONSTANT * mass_current;

            /* compute the new position and velocity of the body */
            double acceleration[2] = {(forces_on_body[0] * DELTA_T) / mass_current, (forces_on_body[1] * DELTA_T) / mass_current};

            double velocity[2] = {velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] + acceleration[0],
                                  velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] + acceleration[1] * DELTA_T};

            double position[2] = {positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] + velocity[0] * DELTA_T,
                                  positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] + velocity[1] * DELTA_T};

            /* update the body */
            velocities[j * SENDED_DATA_SIZE + VELOCITY_X_INDEX] = velocity[0];
            velocities[j * SENDED_DATA_SIZE + VELOCITY_Y_INDEX] = velocity[1];
            positions[j * SENDED_DATA_SIZE + POSITION_X_INDEX] = position[0];
            positions[j * SENDED_DATA_SIZE + POSITION_Y_INDEX] = position[1];
        }
    }

    double end_time = MPI_Wtime();
    if (current_rank == HOST_RANK)
        printf("Time: %f\n", end_time - start_time);

    // free memory
    free(ids);
    free(masses);
    free(recvcounts);
    free(displs);
    free(velocities);
    free(positions);
    free(received_positions);

    MPI_Finalize();
    return 0;
}